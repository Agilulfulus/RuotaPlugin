#include <Windows.h>
#include <atlstr.h>
#include <string>
#include "Ruota/Ruota.h"
#include "../../API/RainmeterAPI.h"

/* 
[MeasureName]
Measure=Plugin
Plugin=RuotaScript
ScriptFile=MyScript.ruo
*/

const char * rm_compiled = {
#include "Rainmeter.ruo"
};

RuotaWrapper * rw;

void * rm_base;

std::wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

std::vector<SP_Memory> __notice(std::vector<SP_Memory> args) {
	RmLog(LOG_NOTICE, (LPCWSTR)(s2ws(args[0]->toString())).c_str());
	return { new_memory() };
}

std::vector<SP_Memory> __bang(std::vector<SP_Memory> args) {
	RmExecute(RmGetSkin(rm_base), (LPCWSTR)(s2ws(args[0]->toString())).c_str());
	return { new_memory() };
}

std::vector<SP_Memory> __getvar(std::vector<SP_Memory> args) {
	LPCWSTR myVar = RmReplaceVariables(rm_base, (LPCWSTR)(s2ws("#" + args[0]->toString() + "#")).c_str());
	std::string value = CW2A(myVar);
	return { new_memory(value) };
}

std::vector<SP_Memory> __getoption(std::vector<SP_Memory> args) {
	LPCWSTR myVar = RmReadString(rm_base, (LPCWSTR)(s2ws(args[0]->toString())).c_str(), L"");
	std::string value = CW2A(myVar);
	return { new_memory(value) };
}


struct Measure
{
	Measure() {}
};

PLUGIN_EXPORT void Initialize(void** data, void* rm)
{
	rm_base = rm;
	Measure* measure = new Measure;
	*data = measure;
	rw = new RuotaWrapper((std::string)CW2A(RmPathToAbsolute(rm, L"@Resources\\")));
	LPCWSTR filename_raw = RmReadString(rm, L"ScriptFile", L"");
	Interpreter::addEmbed("rm.notice", &__notice);
	Interpreter::addEmbed("rm.bang", &__bang);
	Interpreter::addEmbed("rm.getvar", &__getvar);
	Interpreter::addEmbed("rm.getoption", &__getoption);
	rw->runLine(rm_compiled);
	try {
		std::string content = "";
		std::string line;
		std::ifstream myfile(filename_raw);
		if (myfile.is_open()){
			while (getline(myfile, line))
				content += line + "\n";
			myfile.close();
		}
		rw->runLine(content);
		rw->runLine("Initialize();");
	}
	catch (std::runtime_error &e) {
		RmLog(LOG_ERROR, (LPCWSTR)(s2ws(e.what())).c_str());
	}
}

PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue)
{
	Measure* measure = (Measure*)data;
}

PLUGIN_EXPORT double Update(void* data)
{
	Measure* measure = (Measure*)data;
	try {
		rw->runLine("Update()");
	}
	catch (std::runtime_error &e) {
		RmLog(LOG_ERROR, (LPCWSTR)(s2ws(e.what())).c_str());
	}
	return 0.0;
}

PLUGIN_EXPORT LPCWSTR GetString(void* data)
{
	Measure* measure = (Measure*)data;
	SP_Memory var = rw->runLine("Value()");
	std::wstring val = s2ws(var->toString());
	return (LPCWSTR)val.c_str();
}

PLUGIN_EXPORT void ExecuteBang(void* data, LPCWSTR args)
{
	try {
		rw->runLine((std::string)CW2A(args));
	}
	catch (std::runtime_error &e) {
		RmLog(LOG_ERROR, (LPCWSTR)(s2ws(e.what())).c_str());
	}
	Measure* measure = (Measure*)data;
}

//PLUGIN_EXPORT LPCWSTR (void* data, const int argc, const WCHAR* argv[])
//{
//	Measure* measure = (Measure*)data;
//	return nullptr;
//}

PLUGIN_EXPORT void Finalize(void* data)
{
	Measure* measure = (Measure*)data;
	delete measure;
}
