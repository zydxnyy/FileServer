#pragma once
#include <iostream>
#include <json/json.h>
#include "HttpConnect.h"
#include "util.h"
using namespace std;

Py_Ret verify_account(string email, string password);

Py_Ret getAllProject(string email, string token);

int checkToken(string email, string token);
