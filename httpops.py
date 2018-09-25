# -*- coding: utf-8 -*-


import requests, sys, json

#addr = "172.18.94.114:8000"
addr = "10.132.100.180:9091"

def verify_account(email, password):
	url = 'http://'+addr+'/api/verify_account/' #django api路径
	parms = {
		'email' : email,
		'password' : password
	}
	headers = {
	#	'User-agent' : 'none/ofyourbusiness',
	#    'Spam' : 'Eggs'
	}

	resp = requests.post(url, data=parms, headers=headers)#发送请求

	text = resp.text
	dict = json.loads(text)
	
	error_msg = dict.get('error_msg', '')
	token = dict.get('token', '')
	
	if error_msg != '':
		print error_msg
		return (-1, error_msg.encode('utf-8'))
	else:
		return (0, token)


def get_self_project(email, token):
	url = 'http://'+addr+'/api/get_self_project/' #django api路径
	parms = {
		'email' : email,
		'token' : token
	}
	headers = {
	#	'User-agent' : 'none/ofyourbusiness',
	#    'Spam' : 'Eggs'
	}

	resp = requests.post(url, data=parms, headers=headers)#发送请求
	text = resp.text
	print text
	dict = json.loads(text)
	
	error_msg = dict.get('error_msg','')
	my_projects = dict.get('my_projects', '')
	
	if error_msg != '':
		print error_msg
		return (-1, error_msg)
	else:
		print my_projects
		return (0, my_projects.encode('utf-8'))
		my_projects = json.loads(my_projects)
					
def get_all_project(email, token):
	url = 'http://'+addr+'/api/get_all_project/' #django api路径
	parms = {
		'email' : email,
		'token' : token
	}
	headers = {
	#	'User-agent' : 'none/ofyourbusiness',
	#    'Spam' : 'Eggs'
	}

	resp = requests.post(url, data=parms, headers=headers)#发送请求
	text = resp.text
	#print text.encode('utf-8')
	dict = json.loads(text)
	
	error_msg = dict.get('error_msg','')
	projects = dict.get('projects', '')
	
	if error_msg != '':
		print error_msg
		return (-1, error_msg)
	else:
		# print projects
		return (0, projects.encode('utf-8'))
		projects = json.loads(projects)


def check_token(email, token):
	url = 'http://'+addr+'/api/check_token/' #django api路径
	parms = {
		'email' : email,
		'token' : token
	}
	headers = {
	#	'User-agent' : 'none/ofyourbusiness',
	#    'Spam' : 'Eggs'
	}

	resp = requests.post(url, data=parms, headers=headers)#发送请求

	text = resp.text
	return int(text)
	
	
# def chunks(f, chunk_size=None):
	# """
	# Read the file and yield chunks of ``chunk_size`` bytes (defaults to
	# ``UploadedFile.DEFAULT_CHUNK_SIZE``).
	# """
	# if not chunk_size:
		# chunk_size = 64 * 2 ** 10

	# try:
		# f.seek(0)
	# except (AttributeError, UnsupportedOperation):
		# pass

	# while True:
		# data = f.read(chunk_size)
		# if not data:
			# break
		# yield data
	
	
# def lw_file(file_path, email, token, project_name):
	# with open(file_path, 'rb', encoding = 'utf-8') as lf:
		# with irods_obj.open('r+') as rf:
			# for chunk in chunks(lf):
				# rf.write(chunk)
			# rf.close()
		
#i, token = verify_account('123@qq.com', '12345')
#print get_self_project('123@qq.com', token)
#print check_token('123@qq.com', token)