#include "FileServer.h"
using namespace std;

const string ROOTDIR = "/irods2/";
const string PROTEINDIR = "protein/";
const string DRUGDIR = "drug/";
const string ANIMALDIR = "animal/";
const string METADIR = "metabolomics/";

FileServer::FileServer()
{
	logger = Logger::getLogger();
	logger->log("Starting FileServer...", INFO);
	UDT::startup();
	if (chdir(ROOTDIR.c_str()) == -1) {
		logger->log("Failed to change directory", ERROR);
		perror("");
		exit(-1);
	}
	else {
		logger->log("FileServer Started...", INFO); 
		char   buffer[1024];
		getcwd(buffer, 1024);
		logger->log(buffer, INFO);
	}
}

char* mstrcat(int n, char* dest, ...) {
	va_list args;
	va_start(args, n);
	while (n > 0) {    //通过va_arg(args,int)依次获取参数的值
		strcat(dest, va_arg(args,char*));
		n--;
	}
	va_end(args);
	return dest;
}


FileServer::~FileServer()
{
	UDT::close(serv);

	// use this function to release the UDT library
	UDT::cleanup();

	logger->log("Shutdown...", INFO);
	delete logger;
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

	logger->log("Listening to port 5566", INFO);

	if (0 != getaddrinfo(NULL, service.c_str(), &hints, &res)) {
		Trace << "illegal port number or port is busy.\n" << endl;
		logger->log("PORT is busy", ERROR);
		return false;
	}

	serv = UDT::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if (UDT::ERROR == UDT::bind(serv, res->ai_addr, res->ai_addrlen)) {
		Trace << "bind: " << UDT::getlasterror().getErrorMessage() << endl;
		logger->log("Bind Failed", ERROR);
		return false;
	}

	freeaddrinfo(res);

	Trace << "server is ready at port: " << service << endl;
	logger->log("Server is ready at port 5566", INFO);

	if (UDT::ERROR == UDT::listen(serv, SOMAXCONN))
	{
		Trace << "listen: " << UDT::getlasterror().getErrorMessage() << endl;
		logger->log("Failed to listen", ERROR);
		return 0;
	}

	sockaddr_storage clientaddr;
	int addrlen = sizeof(clientaddr);


	while (true) {
		UDTSOCKET fhandle;
		if (UDT::INVALID_SOCK == (fhandle = UDT::accept(serv, (sockaddr*)&clientaddr, &addrlen))) {
			Trace << "accept: " << UDT::getlasterror().getErrorMessage() << endl;
			logger->log("Accept A invalid socket", WARN);
			return false;
		}
		int timeout = 10000;
		UDT::setsockopt(fhandle, 0, UDT_SNDTIMEO, (char*)&timeout, sizeof(int));
		UDT::setsockopt(fhandle, 0, UDT_RCVTIMEO, (char*)&timeout, sizeof(int));
		char clienthost[NI_MAXHOST];
		char clientservice[NI_MAXSERV];
		getnameinfo((sockaddr *)&clientaddr, addrlen, clienthost, sizeof(clienthost), clientservice, sizeof(clientservice), NI_NUMERICHOST | NI_NUMERICSERV);
		Trace << "new connection: " << clienthost << ":" << clientservice << endl;

		//logger->log("New connection:" + clienthost + ":" + clientservice, INFO);
		//logger->log(mstrcat(3, "New connection", clienthost, ":", clientservice), INFO);

		std::thread t{ &FileServer::work_thread, this, fhandle, string(clienthost), string(clientservice) };
		t.detach();
	}
	logger->log("Out While", WARN);

	return true;
}

void FileServer::work_thread(UDTSOCKET fhandle, string clienthost, string clientservice) {
	string addr = "<" + clienthost + ":" + clientservice + ">";
	char log_buf[1024];
	FileRequest msg;
	FileReply rpy;
	//接收操作
	if (UDT::ERROR == gRecv(fhandle, (char*)&msg, sizeof(msg), 0)) {
		Trace << "recv: " << UDT::getlasterror().getErrorMessage() << endl;
		memset(log_buf, 0, sizeof(log_buf));
		logger->log(mstrcat(3, log_buf, addr.c_str(), "recv:", UDT::getlasterror().getErrorMessage()), ERROR);
		return;
	}

	int rtn; check_token(msg.email, msg.token, rtn);

	if (rtn == -1) {
		Trace << "Server internal error" << endl;
		memset(log_buf, 0, sizeof(log_buf));
		logger->log(mstrcat(2, log_buf, addr.c_str(), "Server Internal Error:"), ERROR);
		rpy.error_flag = SERVER_INTERNAL_ERROR;
		memcpy(rpy.extra, "Server Internal Error", 30);
		rpy.op = msg.op;
		gSend(fhandle, (char*)&rpy, sizeof(rpy), 0);
		return;
	}
	else if (rtn != 0) {
		Trace << "TOKEN_EXPIRE" << endl;
		rpy.error_flag = TOKEN_EXPIRE;
		memcpy(rpy.extra, "Invalid user", 22);
		rpy.op = msg.op;
		gSend(fhandle, (char*)&rpy, sizeof(rpy), 0);
		return;
	}

	Trace << "ID verified" << endl;

	//获取文件目录结构
	if (msg.op == GETDIR) {
		memset(log_buf, 0, sizeof(log_buf));
		logger->log(mstrcat(2, log_buf, addr.c_str(), " Getting Directory"), INFO);
		Trace << "Getting dir" << endl;
		vector<vector<Proj> > projects = getProjects(msg.email, msg.token);
		Json::Value root, rrrrr;
		//查本地文件夹中项目文件夹的文件（通过查询数据库）
		
		//获取Protein项目
		for (uint i = 0; i < projects[0].size(); ++i) {
			string projname = projects[0].at(i).name;
			Json::Value sproj;
			vector<myfile> files = f.queryProject("Protein", getProjId(projname));
			sproj["name"] = projects[0].at(i).name;
			sproj["user"] = projects[0].at(i).uid;
			Json::Value pdata;
			pdata["data"].resize(0);
			
			for (uint j = 0; j < files.size(); ++j) {
				if (files.at(j).status != 1) continue;
				Json::Value sfile;
				sfile["name"] = files.at(j).filename;
				sfile["size"] = (double)files.at(j).filesize;
				sfile["email"] = files.at(j).email;
				sfile["created"] = (double)files.at(j).uploadtime;
				sfile["projname"] = getProjName(files.at(j).projId);
				sfile["fileHash"] = files.at(j).fileHash;
				pdata["data"].append(sfile);
			}
			sproj["pdata"] = pdata;
			root.append(sproj);

		}
		rrrrr["Protein"] = root;
		root.clear();
		//获取Drug项目
		for (int i = 0; i < projects[1].size(); ++i) {
			string projname = projects[1].at(i).name;
			Json::Value sproj;
			vector<myfile> files = f.queryProject("Drug", getProjId(projname));
			sproj["name"] = projects[1].at(i).name;
			sproj["user"] = projects[1].at(i).uid;
			Json::Value pdata;
			pdata["data"].resize(0);

			for (int j = 0; j < files.size(); ++j) {
				if (files.at(j).status != 1) continue;
				Json::Value sfile;
				sfile["name"] = files.at(j).filename;
				sfile["size"] = (double)files.at(j).filesize;
				sfile["email"] = files.at(j).email;
				sfile["created"] = (double)files.at(j).uploadtime;
				sfile["projname"] = getProjName(files.at(j).projId);
				sfile["fileHash"] = files.at(j).fileHash;
				pdata["data"].append(sfile);
			}
			sproj["pdata"] = pdata;
			root.append(sproj);
		}
		rrrrr["Drug"] = root;
		root.clear();
		//获取animal项目
		for (int i = 0; i < projects[2].size(); ++i) {
			string projname = projects[2].at(i).name;
			Json::Value sproj;
			vector<myfile> files = f.queryProject("Animal", getProjId(projname));
			sproj["name"] = projects[2].at(i).name;
			sproj["user"] = projects[2].at(i).uid;
			Json::Value pdata;
			pdata["data"].resize(0);

			for (int j = 0; j < files.size(); ++j) {
				if (files.at(j).status != 1) continue;
				Json::Value sfile;
				sfile["name"] = files.at(j).filename;
				sfile["size"] = (double)files.at(j).filesize;
				sfile["email"] = files.at(j).email;
				sfile["created"] = (double)files.at(j).uploadtime;
				sfile["projname"] = getProjName(files.at(j).projId);
				sfile["fileHash"] = files.at(j).fileHash;
				pdata["data"].append(sfile);
			}
			sproj["pdata"] = pdata;
			root.append(sproj);
		}
		rrrrr["Animal"] = root;
		root.clear();
		//获取Metabolomics项目
		for (int i = 0; i < projects[3].size(); ++i) {
			string projname = projects[2].at(i).name;
			Json::Value sproj;
			vector<myfile> files = f.queryProject("Metabolomics", getProjId(projname));
			sproj["name"] = projects[2].at(i).name;
			sproj["user"] = projects[2].at(i).uid;
			Json::Value pdata;
			pdata["data"].resize(0);

			for (int j = 0; j < files.size(); ++j) {
				if (files.at(j).status != 1) continue;
				Json::Value sfile;
				sfile["name"] = files.at(j).filename;
				sfile["size"] = (double)files.at(j).filesize;
				sfile["email"] = files.at(j).email;
				sfile["created"] = (double)files.at(j).uploadtime;
				sfile["projname"] = getProjName(files.at(j).projId);
				sfile["fileHash"] = files.at(j).fileHash;
				pdata["data"].append(sfile);
			}
			sproj["pdata"] = pdata;
			root.append(sproj);
		}
		rrrrr["Metabolomics"] = root;

		string jsonProjStr;
		json_write(jsonProjStr, rrrrr);
		Trace << jsonProjStr << endl;
		//Trace << jsonProjStr << endl;
		if (UDT::ERROR == gSend(fhandle, (char*)(new int(jsonProjStr.size())), sizeof(int), 0)) {
			memset(log_buf, 0, sizeof(log_buf));
			logger->log(mstrcat(2, log_buf, addr.c_str(), " Send json len failed."), INFO);
			Trace << "send: " << UDT::getlasterror().getErrorMessage() << endl;
			return;
		}
		if (UDT::ERROR == gSend(fhandle, jsonProjStr.c_str(), jsonProjStr.size(), 0)) {
			memset(log_buf, 0, sizeof(log_buf));
			logger->log(mstrcat(2, log_buf, addr.c_str(), " Send json failed."), INFO);
			Trace << "send: " << UDT::getlasterror().getErrorMessage() << endl;
			return;
		}
	}
	//上传
	else if (msg.op == UPLOAD) {
		memset(log_buf, 0, sizeof(log_buf));
		logger->log(mstrcat(2, log_buf, addr.c_str(), "Uploading File"), INFO);
		UDT::TRACEINFO tinfo;
		//获取存储路径
		string typeDir;
		if (string(msg.type) == "Protein") typeDir = PROTEINDIR;
		else if (string(msg.type) == "Drug") typeDir = DRUGDIR;
		else if (string(msg.type) == "Animal") typeDir = ANIMALDIR;
		else if (string(msg.type) == "Metabolomics") typeDir = METADIR;

		string dir = typeDir + string(msg.project_name) + "/";
		Trace << "Dir = " << dir << endl;
		mkdir(dir.c_str(), S_IRWXU);
		//查询上传的文件在数据库中是否有实例
		myfile ufile = f.queryFile(msg.type, getProjId(msg.project_name), msg.file_name);
		//如果文件已经存在记录
		if (ufile.valid()) {
			//文件已经完成，返回文件已存在错误
			if (ufile.status == 1) {
				rpy.error_flag = FILE_EXISTS;
				rpy.op = UPLOAD;
				memcpy(rpy.extra, "File already exists", 20);
				if (UDT::ERROR == gSend(fhandle, (char*)&rpy, sizeof(rpy), 0)) {
					Trace << "sendmsg: " << UDT::getlasterror().getErrorMessage() << endl;
					return;
				}
				UDT::close(fhandle);
				return;
			}
			else if (ufile.status == 2) {
				rpy.error_flag = FILE_EXISTS;
				rpy.op = UPLOAD;
				memcpy(rpy.extra, "File already exists", 20);
				if (UDT::ERROR == gSend(fhandle, (char*)&rpy, sizeof(rpy), 0)) {
					Trace << "sendmsg: " << UDT::getlasterror().getErrorMessage() << endl;
					return;
				}
				f.update(msg.type, getProjId(msg.project_name), msg.file_name, 1, ufile.offset);
				UDT::close(fhandle);
				return;
			}
			//如果文件未完成，那么判断未完成文件是否同一文件
			else {
				//如果是同一文件，允许继续
				if (string(msg.fileHash) == ufile.fileHash) {
					Trace << "File continue;" << endl;
				}
				//如果是不同文件，证明同名，拒绝上传
				else {
					Trace << "File exists same name;" << endl;
					rpy.error_flag = FILE_NAME_DULPLICTAION;
					rpy.op = UPLOAD;
					memcpy(rpy.extra, "File already exists", 20);
					if (UDT::ERROR == gSend(fhandle, (char*)&rpy, sizeof(rpy), 0)) {
						Trace << "sendmsg: " << UDT::getlasterror().getErrorMessage() << endl;
						return;
					}
					UDT::close(fhandle);
					return;
				}
			}
		}
		//如果文件不存在，插入文件记录
		else {
			Trace << "Insert filerecord" << endl;
			if (f.insertFile(msg.type, msg.email, msg.file_name, msg.file_size, dir + string(msg.file_name), msg.fileHash, 0, 0, getProjId(msg.project_name))) {
				Trace << "Insert done" << endl;
			}
			else {
				Trace << "Insert failed" << endl;
				return;
			}
		}
		// recv the file
		int64_t offset = ufile.offset;
		Trace << dir + string(msg.file_name) + ".ft.nc" << endl;
		fstream ofs(dir + string(msg.file_name) + ".ft.nc", ios::out | ios::binary | ios::app);
		if (!ofs.is_open()) {
			Trace << "Can't open " << dir + string(msg.file_name) + ".ft.nc" << endl;
			memset(log_buf, 0, sizeof(log_buf));
			logger->log(mstrcat(2, log_buf, addr.c_str(), " File open failed."), INFO);
			rpy.op = UPLOAD;
			rpy.error_flag = SERVER_INTERNAL_ERROR;
			memcpy(rpy.extra, "File open failed", 20);
			// send file size information
			if (UDT::ERROR == gSend(fhandle, (char*)&rpy, sizeof(rpy), 0)) {
				Trace << "send: " << UDT::getlasterror().getErrorMessage() << endl;
				return;
			}
			UDT::close(fhandle);
			return;
		}
		int64_t size = msg.file_size;
		UDT::perfmon(fhandle, &tinfo);

		Trace << msg.file_name << " " << msg.file_size << " " << offset << endl;

		//允许上传
		rpy.op = UPLOAD; rpy.offset = offset;
		if (UDT::ERROR == gSend(fhandle, (char*)&rpy, sizeof(rpy), 0)) {
			Trace << "sendmsg: " << UDT::getlasterror().getErrorMessage() << endl;
			return;
		}
		if (UDT::ERROR == UDT::recvfile(fhandle, ofs, offset, size - offset)) {
			Trace << "recvfile: " << UDT::getlasterror().getErrorMessage() << " offset = " << offset << endl;
			ofs.close();
			f.update(msg.type, getProjId(msg.project_name), msg.file_name, 0, offset);
			return;
		}
		ofs.close();
		rename((dir + string(msg.file_name) + ".ft.nc").c_str(), (dir + string(msg.file_name)).c_str());
		string fileHash = hashFile((dir + string(msg.file_name)).c_str());
		Trace << "Hash = " << fileHash << " " << " uploadHash = " << msg.fileHash << endl;

		//哈希校验失败，删除本地文件及数据库数据
		if (fileHash != string(msg.fileHash)) {
			f.deleteFile(msg.type, getProjId(msg.project_name), msg.file_name);
			remove((dir + string(msg.file_name) + ".ft.nc").c_str());
			rpy.error_flag = FILE_HASH_FAULT;
			memcpy(rpy.extra, "File hash check failed", 30);
			// send file size information
			if (UDT::ERROR == gSend(fhandle, (char*)&rpy, sizeof(rpy), 0)) {
				Trace << "send: " << UDT::getlasterror().getErrorMessage() << endl;
				return;
			}
			UDT::close(fhandle);
			return;
		}
		//文件上传成功，更新文件在数据库中的状态
		f.update(msg.type, getProjId(msg.project_name), msg.file_name, 1, offset);
		rpy.error_flag = MY_NO_ERROR;
		memcpy(rpy.extra, "File send succ", 30);
		// send file size information
		if (UDT::ERROR == gSend(fhandle, (char*)&rpy, sizeof(rpy), 0)) {
			Trace << "send: " << UDT::getlasterror().getErrorMessage() << endl;
			return;
		}
		UDT::perfmon(fhandle, &tinfo);
		Trace << "speed = " << tinfo.mbpsRecvRate << "Mbits/sec" << " offset = " << offset << endl;
	}
	//下载
	else if (msg.op == DOWNLOAD) {
		memset(log_buf, 0, sizeof(log_buf));
		logger->log(mstrcat(2, log_buf, addr.c_str(), "Downloading File"), INFO);
		myfile ufile = f.queryFile(msg.type, getProjId(msg.project_name), msg.file_name);
		if (!ufile.valid() || ufile.status == 0) {
			Trace << "File Not exists" << endl;
			rpy.error_flag = FILE_NOT_EXISTS;
			rpy.op = DOWNLOAD;
			memcpy(rpy.extra, "File not exists1", 20);
			memset(log_buf, 0, sizeof(log_buf));
			logger->log(mstrcat(2, log_buf, addr.c_str(), " File not exists1."), INFO);
			if (UDT::ERROR == gSend(fhandle, (char*)&rpy, sizeof(rpy), 0)) {
				Trace << "sendmsg: " << UDT::getlasterror().getErrorMessage() << endl;
				return;
			}
			UDT::close(fhandle);
			return;
		}
		else {
			Trace << "File exists, allow to download " << endl;
			Trace << "File = " << ufile << endl;
		}
		UDT::TRACEINFO tinfo;
		string filepath = ufile.filepath;
		// open the file
		fstream ifs(filepath, ios::in | ios::binary);
		Trace << filepath << endl;
		if (!ifs) {
			rpy.op = DOWNLOAD;
			rpy.error_flag = FILE_NOT_EXISTS;
			memcpy(rpy.extra, "File not exists2", 20);
			memset(log_buf, 0, sizeof(log_buf));
			logger->log(mstrcat(2, log_buf, addr.c_str(), " File not exists2."), INFO);
			// send file size information
			if (UDT::ERROR == gSend(fhandle, (char*)&rpy, sizeof(rpy), 0)) {
				Trace << "send: " << UDT::getlasterror().getErrorMessage() << endl;
				return;
			}
			UDT::close(fhandle);
			Trace << "File open failed" << endl;
			return;
		}
		ifs.seekg(0, ios::end);
		int64_t size = ifs.tellg();
		ifs.seekg(0, ios::beg);
		memcpy(rpy.fileHash, ufile.fileHash.c_str(), ufile.fileHash.size());
		rpy.op = DOWNLOAD;
		rpy.file_size = size;
		rpy.offset = 0;
		Trace << "File hash = " << rpy.fileHash << endl;
		// send file size information
		if (UDT::ERROR == gSend(fhandle, (char*)&rpy, sizeof(rpy), 0)) {
			Trace << "sendRpy: " << UDT::getlasterror().getErrorMessage() << endl;
			return;
		}

		UDT::perfmon(fhandle, &tinfo);

		// send the file
		char buffer[CHUNK_SIZE];
		int64_t offset = msg.offset;
		ifs.seekg(offset, ios::beg);
		size_t remainSize = size - offset;
		//if (UDT::ERROR == UDT::sendfile(fhandle, ifs, offset, remainSize)) {
		//	UDT::close(fhandle);
		//	ifs.close();
		//	Trace << "sendfile: " << UDT::getlasterror().getErrorMessage() << endl;
		//	return;
		//}
		int ret;
		while (remainSize) {
			if (remainSize >= CHUNK_SIZE) {
				ifs.read(buffer, CHUNK_SIZE);
				if (UDT::ERROR == (ret = gSend(fhandle, buffer, CHUNK_SIZE, 0))) {
					UDT::close(fhandle);
					ifs.close();
					Trace << "sendfile: " << UDT::getlasterror().getErrorMessage() << endl;
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
					Trace << "sendfile: " << UDT::getlasterror().getErrorMessage() << endl;
					return;
				}
				offset += remainSize;
				remainSize -= remainSize;
			}
			//Trace << remainSize << " ret = " << ret << endl;
		}
		ifs.close();
		UDT::perfmon(fhandle, &tinfo);
		Trace << "speed = " << tinfo.mbpsSendRate / 8 << "MBs/sec" << endl;

	}
	else if (msg.op == DEL) {
		memset(log_buf, 0, sizeof(log_buf));
		logger->log(mstrcat(2, log_buf, addr.c_str(), "Deleting File"), INFO);
		myfile ufile = f.queryFile(msg.type, getProjId(msg.project_name), msg.file_name);
		//如果文件不合法
		if (!ufile.valid() || ufile.status ==0) {
			rpy.error_flag = FILE_NOT_EXISTS;
			rpy.op = DEL;
			memcpy(rpy.extra, "File not exists", 20);
			gSend(fhandle, (char*)&rpy, sizeof(rpy), 0);
			UDT::close(fhandle);
			return;
		}
		//验证文件是否被用户拥有
		string path = ufile.filepath;
		Trace << "DEL path = " << path << endl;
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
		if (remove(path.c_str()) != 0) {
			rpy.error_flag = SERVER_INTERNAL_ERROR;
			rpy.op = DEL;
			memcpy(rpy.extra, "DELETE Failed", 20);
			Trace << "Remove failed" << endl;
			gSend(fhandle, (char*)&rpy, sizeof(rpy), 0);
		}
		else {
			f.deleteFile(msg.type, getProjId(msg.project_name), msg.file_name);
			rpy.error_flag = MY_NO_ERROR;
			rpy.op = DEL;
			memcpy(rpy.extra, "DELETE SUCCESS", 20);
			gSend(fhandle, (char*)&rpy, sizeof(rpy), 0);
		}
	}
	///////////////////////////
	UDT::close(fhandle);
	Trace << "Done thread" << endl;
	memset(log_buf, 0, sizeof(log_buf));
	logger->log(mstrcat(2, log_buf, addr.c_str(), "Exit"), INFO);
}

vector<vector<Proj> > FileServer::getProjects(const string& email, const string& token)
{
	vector<Proj> proteinProjects;
	vector<Proj> drugProjects;
	vector<Proj> animalProjects;
	vector<Proj> metaProjects;
	vector<vector<Proj> > retv;
	Py_Ret ret = get_all_project(email, token);
	if (ret.status != 0) Trace << "ERROR" << ret.str << endl;
	else {
		char* result = (char*)ret.str.c_str();
		Trace << result << endl;
		Json::Value vvv;
		if (json_parse(result, vvv)) {
			Json::Value root;
			root.clear();
			json_parse(vvv["Protein"].asCString(), root);
			Trace << root << endl;
			for (uint i = 0; i < root.size(); ++i) {
				Proj proj;
				proj.name = root[i]["name"].asCString();
				proj.uid = root[i]["user"].asInt();
				proteinProjects.push_back(proj);
			}
			root.clear();
			json_parse(vvv["Drug"].asCString(), root);
			for (uint i = 0; i < root.size(); ++i) {
				Proj proj;
				proj.name = root[i]["name"].asCString();
				proj.uid = root[i]["user"].asInt();
				drugProjects.push_back(proj);
			}
			root.clear();
			json_parse(vvv["Animal"].asCString(), root);
			for (uint i = 0; i < root.size(); ++i) {
				Proj proj;
				proj.name = root[i]["name"].asCString();
				proj.uid = root[i]["user"].asInt();
				animalProjects.push_back(proj);
			}
			root.clear();
			json_parse(vvv["Metabolomics"].asCString(), root);
			for (uint i = 0; i < root.size(); ++i) {
				Proj proj;
				proj.name = root[i]["name"].asCString();
				proj.uid = root[i]["user"].asInt();
				metaProjects.push_back(proj);
			}
			retv.push_back(proteinProjects);
			retv.push_back(drugProjects);
			retv.push_back(animalProjects);
			retv.push_back(metaProjects);
		}
	}
	return retv;
}
