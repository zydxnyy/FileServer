#include "FileServer.h"
using namespace std;

const string DIR = "/home/webadmin/projects/linuxServer/";
const string RECVDIR = "FileRecv/";

FileServer::FileServer()
{
	UDT::startup();
	if (chdir(DIR.c_str()) == -1) {
		cout << "Change failed" << endl;
		perror("");
		exit(-1);
	}
	mkdir(RECVDIR.c_str(), S_IRWXU);
}


FileServer::~FileServer()
{

	UDT::close(serv);

	// use this function to release the UDT library
	UDT::cleanup();
}

bool FileServer::start()
{
	addrinfo hints;
	addrinfo* res;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	string service("5566");

	if (0 != getaddrinfo(NULL, service.c_str(), &hints, &res)) {
		cout << "illegal port number or port is busy.\n" << endl;
		return false;
	}

	serv = UDT::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if (UDT::ERROR == UDT::bind(serv, res->ai_addr, res->ai_addrlen)) {
		cout << "bind: " << UDT::getlasterror().getErrorMessage() << endl;
		return false;
	}

	freeaddrinfo(res);

	cout << "server is ready at port: " << service << endl;

	if (UDT::ERROR == UDT::listen(serv, SOMAXCONN))
	{
		cout << "listen: " << UDT::getlasterror().getErrorMessage() << endl;
		return 0;
	}

	sockaddr_storage clientaddr;
	int addrlen = sizeof(clientaddr);


	while (true) {
		UDTSOCKET fhandle;
		if (UDT::INVALID_SOCK == (fhandle = UDT::accept(serv, (sockaddr*)&clientaddr, &addrlen))) {
			cout << "accept: " << UDT::getlasterror().getErrorMessage() << endl;
			return false;
		}
		int timeout = 10000;
		UDT::setsockopt(fhandle, 0, UDT_SNDTIMEO, (char*)&timeout, sizeof(int));
		UDT::setsockopt(fhandle, 0, UDT_RCVTIMEO, (char*)&timeout, sizeof(int));
		char clienthost[NI_MAXHOST];
		char clientservice[NI_MAXSERV];
		getnameinfo((sockaddr *)&clientaddr, addrlen, clienthost, sizeof(clienthost), clientservice, sizeof(clientservice), NI_NUMERICHOST | NI_NUMERICSERV);
		cout << "new connection: " << clienthost << ":" << clientservice << endl;

		std::thread t{ &FileServer::work_thread, this, fhandle };
		t.detach();
	}

	return true;
}

void FileServer::work_thread(UDTSOCKET fhandle) {
	FileRequest msg;
	FileReply rpy;
	//接收操作
	if (UDT::ERROR == gRecv(fhandle, (char*)&msg, sizeof(msg), 0)) {
		cout << "recv: " << UDT::getlasterror().getErrorMessage() << endl;
		return;
	}

	int rtn; check_token(msg.email, msg.token, rtn);
	cout << "rtn = " << rtn << endl;
	if (rtn == -1) {
		rpy.error_flag = SERVER_INTERNAL_ERROR;
		memcpy(rpy.extra, "Server Internal Error", 30);
		rpy.op = msg.op;
		gSend(fhandle, (char*)&rpy, sizeof(rpy), 0);
		return;
	}
	else if (rtn != 0) {
		rpy.error_flag = TOKEN_EXPIRE;
		memcpy(rpy.extra, "Invalid user", 22);
		rpy.op = msg.op;
		gSend(fhandle, (char*)&rpy, sizeof(rpy), 0);
		return;
	}

	cout << "ID verified" << endl;

	//获取文件目录结构
	if (msg.op == GETDIR) {
		vector<Proj> projects = getProjects(msg.email, msg.token);
		Json::Value root;
		//查本地文件夹中项目文件夹的文件（通过查询数据库）
		for (int i = 0; i < projects.size(); ++i) {
			string projname = projects.at(i).name;
			Json::Value sproj;
			vector<myfile> files = f.queryProject(projname);
			sproj["name"] = projects.at(i).name;
			sproj["user"] = projects.at(i).uid;
			Json::Value pdata;
			pdata["data"].resize(0);

			for (int j = 0; j < files.size(); ++j) {
				if (files.at(j).state == 0) continue;
				Json::Value sfile;
				sfile["name"] = files.at(j).filename;
				sfile["size"] = (double)files.at(j).filesize;
				sfile["email"] = files.at(j).email;
				sfile["created"] = (double)files.at(j).uploadtime;
				sfile["projname"] = files.at(j).projname;
				sfile["fileHash"] = files.at(j).fileHash;
				pdata["data"].append(sfile);
			}
			sproj["pdata"] = pdata;
			root.append(sproj);
		}
		string jsonProjStr;
		json_write(jsonProjStr, root);
		//cout << jsonProjStr << endl;
		if (UDT::ERROR == gSend(fhandle, (char*)(new int(jsonProjStr.size())), sizeof(int), 0)) {
			cout << "send: " << UDT::getlasterror().getErrorMessage() << endl;
			return;
		}
		if (UDT::ERROR == gSend(fhandle, jsonProjStr.c_str(), jsonProjStr.size(), 0)) {
			cout << "send: " << UDT::getlasterror().getErrorMessage() << endl;
			return;
		}
	}
	//上传
	else if (msg.op == UPLOAD) {
		UDT::TRACEINFO trace;
		//获取存储路径
		string dir = RECVDIR + string(msg.email) + "/" + string(msg.project_name) + "/";
		mkdir((RECVDIR).c_str(), S_IRWXU);
		mkdir((RECVDIR + string(msg.email)).c_str(), S_IRWXU);
		//perror("");
		mkdir(dir.c_str(), S_IRWXU);
		//查询上传的文件在数据库中是否有实例
		myfile ufile = f.queryFile(msg.project_name, msg.file_name);
		//如果文件已经存在记录
		if (ufile.valid()) {
			//文件已经完成，返回文件已存在错误
			if (ufile.state == 1) {
				rpy.error_flag = FILE_EXISTS;
				rpy.op = UPLOAD;
				memcpy(rpy.extra, "File already exists", 20);
				if (UDT::ERROR == gSend(fhandle, (char*)&rpy, sizeof(rpy), 0)) {
					cout << "sendmsg: " << UDT::getlasterror().getErrorMessage() << endl;
					return;
				}
				UDT::close(fhandle);
				return;
			}
			//如果文件未完成，那么判断未完成文件是否同一文件
			else {
				//如果是同一文件，允许继续
				if (msg.file_size == ufile.filesize) {
					cout << "File continue;" << endl;
				}
				//如果是不同文件，证明同名，拒绝上传
				else {
					cout << "File exists same name;" << endl;
					rpy.error_flag = FILE_NAME_DULPLICTAION;
					rpy.op = UPLOAD;
					memcpy(rpy.extra, "File already exists", 20);
					if (UDT::ERROR == gSend(fhandle, (char*)&rpy, sizeof(rpy), 0)) {
						cout << "sendmsg: " << UDT::getlasterror().getErrorMessage() << endl;
						return;
					}
					UDT::close(fhandle);
					return;
				}
			}
		}
		//如果文件不存在，插入文件记录
		else {
			cout << "Insert filerecord" << endl;
			f.insertFile(msg.project_name, msg.file_name, msg.file_size, 0, 0, dir+string(msg.file_name), msg.email, msg.fileHash);
		}
		// recv the file
		int64_t offset = ufile.offset;
		cout << dir + string(msg.file_name) + ".ft.nc" << endl;
		fstream ofs(dir + string(msg.file_name) + ".ft.nc", ios::out | ios::binary | ios::app);
		if (!ofs.is_open()) {
			cout << "Can't open " << dir + string(msg.file_name) + ".ft.nc" << endl;
			rpy.op = UPLOAD;
			rpy.error_flag = SERVER_INTERNAL_ERROR;
			memcpy(rpy.extra, "File open failed", 20);
			// send file size information
			if (UDT::ERROR == gSend(fhandle, (char*)&rpy, sizeof(rpy), 0)) {
				cout << "send: " << UDT::getlasterror().getErrorMessage() << endl;
				return;
			}
			UDT::close(fhandle);
			return;
		}
		int64_t size = msg.file_size;
		UDT::perfmon(fhandle, &trace);

		cout << msg.file_name << " " << msg.file_size << " " << offset << endl;

		//允许上传
		rpy.op = UPLOAD; rpy.offset = offset;
		if (UDT::ERROR == gSend(fhandle, (char*)&rpy, sizeof(rpy), 0)) {
			cout << "sendmsg: " << UDT::getlasterror().getErrorMessage() << endl;
			return;
		}
		if (UDT::ERROR == UDT::recvfile(fhandle, ofs, offset, size - offset)) {
			cout << "recvfile: " << UDT::getlasterror().getErrorMessage() << " offset = " << offset << endl;
			ofs.close();
			f.update(msg.project_name, msg.file_name, 0, offset);
			return;
		}
		ofs.close();
		rename((dir + string(msg.file_name) + ".ft.nc").c_str(), (dir + string(msg.file_name)).c_str());
		string fileHash = hashFile((dir + string(msg.file_name)).c_str());
		cout << "Hash = " << fileHash << " " << " uploadHash = " << msg.fileHash << endl;

		//哈希校验失败，删除本地文件及数据库数据
		if (fileHash != string(msg.fileHash)) {
			f.deleteFile(msg.project_name, msg.file_name);
			remove((dir + string(msg.file_name) + ".ft.nc").c_str());
			rpy.error_flag = FILE_HASH_FAULT;
			memcpy(rpy.extra, "File hash check failed", 30);
			// send file size information
			if (UDT::ERROR == gSend(fhandle, (char*)&rpy, sizeof(rpy), 0)) {
				cout << "send: " << UDT::getlasterror().getErrorMessage() << endl;
				return;
			}
			UDT::close(fhandle);
			return;
		}
		//文件上传成功，更新文件在数据库中的状态
		f.update(msg.project_name, msg.file_name, 1, offset);
		rpy.error_flag = MY_NO_ERROR;
		memcpy(rpy.extra, "File send succ", 30);
		// send file size information
		if (UDT::ERROR == gSend(fhandle, (char*)&rpy, sizeof(rpy), 0)) {
			cout << "send: " << UDT::getlasterror().getErrorMessage() << endl;
			return;
		}
		UDT::perfmon(fhandle, &trace);
		cout << "speed = " << trace.mbpsRecvRate << "Mbits/sec" << " offset = " << offset << endl;
	}
	//下载
	else if (msg.op == DOWNLOAD) {
		myfile ufile = f.queryFile(msg.project_name, msg.file_name);
		if (!ufile.valid() || ufile.state == 0) {
			cout << "File Not exists" << endl;
			rpy.error_flag = FILE_NOT_EXISTS;
			rpy.op = DOWNLOAD;
			memcpy(rpy.extra, "File not exists", 20);
			if (UDT::ERROR == gSend(fhandle, (char*)&rpy, sizeof(rpy), 0)) {
				cout << "sendmsg: " << UDT::getlasterror().getErrorMessage() << endl;
				return;
			}
			UDT::close(fhandle);
			return;
		}
		else {
			cout << "File exists, allow to download " << endl;
			cout << "File = " << ufile << endl;
		}
		UDT::TRACEINFO trace;
		string filepath = ufile.path;
		// open the file
		fstream ifs(filepath, ios::in | ios::binary);
		cout << filepath << endl;
		if (!ifs) {
			rpy.op = DOWNLOAD;
			rpy.error_flag = FILE_NOT_EXISTS;
			memcpy(rpy.extra, "File not exists", 20);
			// send file size information
			if (UDT::ERROR == gSend(fhandle, (char*)&rpy, sizeof(rpy), 0)) {
				cout << "send: " << UDT::getlasterror().getErrorMessage() << endl;
				return;
			}
			UDT::close(fhandle);
			cout << "File open failed" << endl;
			return;
		}
		ifs.seekg(0, ios::end);
		int64_t size = ifs.tellg();
		ifs.seekg(0, ios::beg);
		memcpy(rpy.fileHash, ufile.fileHash.c_str(), ufile.fileHash.size());
		rpy.op = DOWNLOAD;
		rpy.file_size = size;
		rpy.offset = 0;
		cout << "File hash = " << rpy.fileHash << endl;
		// send file size information
		if (UDT::ERROR == gSend(fhandle, (char*)&rpy, sizeof(rpy), 0)) {
			cout << "sendRpy: " << UDT::getlasterror().getErrorMessage() << endl;
			return;
		}

		UDT::perfmon(fhandle, &trace);

		// send the file
		char buffer[CHUNK_SIZE];
		int64_t offset = msg.offset;
		ifs.seekg(offset, ios::beg);
		size_t remainSize = size - offset;
		//if (UDT::ERROR == UDT::sendfile(fhandle, ifs, offset, remainSize)) {
		//	UDT::close(fhandle);
		//	ifs.close();
		//	cout << "sendfile: " << UDT::getlasterror().getErrorMessage() << endl;
		//	return;
		//}
		int ret;
		while (remainSize) {
			if (remainSize >= CHUNK_SIZE) {
				ifs.read(buffer, CHUNK_SIZE);
				if (UDT::ERROR == (ret = gSend(fhandle, buffer, CHUNK_SIZE, 0))) {
					UDT::close(fhandle);
					ifs.close();
					cout << "sendfile: " << UDT::getlasterror().getErrorMessage() << endl;
					return;
				}
				offset += CHUNK_SIZE;
				remainSize -= CHUNK_SIZE;
			}
			else {
				ifs.read(buffer, remainSize);
				if (UDT::ERROR == (ret = gSend(fhandle, buffer, remainSize, 0))) {
					UDT::close(fhandle);
					ifs.close();
					cout << "sendfile: " << UDT::getlasterror().getErrorMessage() << endl;
					return;
				}
				offset += remainSize;
				remainSize -= remainSize;
			}
			//cout << remainSize << " ret = " << ret << endl;
		}
		ifs.close();
		UDT::perfmon(fhandle, &trace);
		cout << "speed = " << trace.mbpsSendRate / 8 << "MBs/sec" << endl;

	}
	else if (msg.op == DEL) {
		myfile ufile = f.queryFile(msg.project_name, msg.file_name);
		//如果文件不合法
		if (!ufile.valid() || ufile.state==0) {
			rpy.error_flag = FILE_NOT_EXISTS;
			rpy.op = DEL;
			memcpy(rpy.extra, "File not exists", 20);
			gSend(fhandle, (char*)&rpy, sizeof(rpy), 0);
			UDT::close(fhandle);
			return;
		}
		//验证文件是否被用户拥有
		string path = ufile.path;
		if (string(msg.email) != ufile.email) {
			rpy.error_flag = USER_AUTH_FAILED;
			rpy.op = DEL;
			memcpy(rpy.extra, "User not allowed", 20);
			gSend(fhandle, (char*)&rpy, sizeof(rpy), 0);
			UDT::close(fhandle);
			return;
		}
		//
		//删除文件，删除本地文件以及数据库数据
		if (remove(path.c_str())) {
			rpy.error_flag = SERVER_INTERNAL_ERROR;
			rpy.op = DEL;
			memcpy(rpy.extra, "DELETE Failed", 20);
			gSend(fhandle, (char*)&rpy, sizeof(rpy), 0);
		}
		else {
			f.deleteFile(msg.project_name, msg.file_name);
			rpy.error_flag = MY_NO_ERROR;
			rpy.op = DEL;
			memcpy(rpy.extra, "DELETE SUCCESS", 20);
			gSend(fhandle, (char*)&rpy, sizeof(rpy), 0);
		}
	}
	///////////////////////////
	UDT::close(fhandle);
	cout << "Done thread" << endl;
}

vector<Proj> FileServer::getProjects(const string& email, const string& token)
{
	vector<Proj> v;
	Py_Ret ret = get_all_project(email, token);
	if (ret.status != 0) cout << "ERROR" << ret.str << endl;
	else {
		char* result = (char*)ret.str.c_str();
		Json::Value root;
		if (json_parse(result, root)) {
			for (int i = 0; i < root.size(); ++i) {
				Proj proj;
				proj.name = root[i]["name"].asCString();
				proj.uid = root[i]["user"].asInt();
				v.push_back(proj);
			}
		}
	}
	return v;
}
