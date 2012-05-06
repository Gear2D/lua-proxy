#include <iostream>
#include <vector>
#include <list>
#include <stack>
#include <algorithm>

#include "LuaBase.h"
#include "LuaArg.h"
#include "LuaProxy.h"


using namespace gear2d;
using namespace gear2d::component;

LuaBase::LuaBase(std::string name){
	L = lua_open();
	luaL_openlibs(L);
	pushTypes();
	registerStaticFunctions();

	//create a pointer to be used when calling methods
	lua_pushlightuserdata(L, this);
	lua_setglobal(L,THIS_AS_LIGHTUSERDATA);

	//TODO review where the file should be located within g2d install
	//int error = luaL_loadfile(L, G2D_LUA_HEADER);
	int error = luaL_dostring(L, G2D_COMPONENT);
	if (error) std::cout<<"Error while opening Gear2D lua functions" << "("<<error<<")"<<std::endl;
	else lua_pcall(L,0,0,0);

}

LuaBase::~LuaBase(){
	//TODO look for destructors in lua files
	lua_close(L);
}

component::type LuaBase::type() {
	return "basic";
}
component::family LuaBase::family() { 
	return "proxy-lua";
}

//TODO actually read dependencies
std::string LuaBase::depends() {
	return "";
}

void LuaBase::handle(parameterbase::id pid, base* lastwrite, object::id owner) {
	std::list<std::string>::const_iterator it;
	for (it = names.begin(); it!=names.end(); it++){
		if (it->length()){
			lua_getfield(L, LUA_GLOBALSINDEX, it->substr(0,it->find("/")).c_str());
			lua_getfield(L, -1, G2D_LUA_HANDLER_TABLE);
			lua_pushstring(L, pid.c_str());
			lua_rawget(L, -2);
			if (lua_isfunction(L, -1)) lua_pcall(L,0,0,0);
			lua_pop(L,3);
		}
	}
}
void LuaBase::setup(object::signature & si) {
	parseComponentNames(si[LUA_COMPONENTS_SIGNATURE_KEY]);
	compath = si["compath"];
	std::list<std::string> notFound; //TODO list?
	std::list<std::string>::iterator it;
	for (it = names.begin(); it!=names.end(); it++){
		const std::list<std::string> &loaded = loadFile(*it);
		components.insert(components.end(), loaded.begin(), loaded.end());
	}
	components.unique(); //TODO also remove non-neighbour duplicates
	
	//call setup for all succesfully loaded components
	for (it = components.begin(); it!=components.end(); it++){
		callMethod(it->c_str(),"setup");
	}
}

std::list<std::string> LuaBase::loadFile(std::string filename){
	std::list<std::string> loaded;
	int error = luaL_loadfile(L, (filename + std::string(".lua")).c_str());
	std::cout<<"Loading "<<(filename + std::string(".lua")).c_str()<<std::endl;
	std::cout<<"Error "<<error<<std::endl;
	if (error){
		std::cout<<"Error while opening file: "<< filename << "("<<error<<")"<<std::endl;
		//notFound.insert(*it);
	}else{
		std::vector<std::string> family_type;
		split(family_type, filename, '/');
		lua_newtable(L);
		lua_setglobal(L, family_type[0].c_str());
		LuaProxy *lp = new LuaProxy(L, family_type[0],family_type[1]);
		//owner->attach(lp);
		loaded.push_back(family_type[0]);
		
		lua_getglobal(L, "depends");
		std::string depends(lua_tostring(L,-1));
		std::list<std::string>::iterator it = loaded.begin();
		if(depends!=std::string("")){
			std::list<std::string> dependencies;
			split(dependencies, depends, ' ');
			while (!dependencies.empty()) {
				component::selector dependency = dependencies.front();
				if (!owner->component(dependency.family)){
					std::cout<<family_type[0]<<" already loaded"<<std::endl;
					try{
						owner->attach(build(dependency));
						//throw evil();
					}catch(evil leo){
						std::cout<<leo.what()<<std::endl;
						std::cout<<"GEAR2D did not load component: "<<dependencies.front()<<std::endl;
						const std::list<std::string> &loadedDependencies = loadFile(dependencies.front());
						loaded.insert(it, loadedDependencies.begin(), loadedDependencies.end());
					}
				}
				dependencies.pop_front();
			}
			
		}
		lua_pop(L,1);
		
	}
	
	return loaded;
}

void LuaBase::update(timediff dt) {
	LuaNumber luadt(dt);
	std::vector<LuaArg*> args(1,&luadt);
	std::list<std::string>::const_iterator it;
	for (it = components.begin(); it!=components.end(); it++){
		//std::cout<<"Updating component "<<*it<<std::endl;
		callMethod(it->c_str(), "update", args);
	}
}

/************************
 * G2D Helper Functions *
 ************************/
int LuaBase::parseComponentNames(std::string depends){
	int count = 0;
	if (depends != "") {
		std::list<std::string> dependencies;
		split(names, depends, ' ');
		std::list<std::string>::const_iterator it;
		for (it = names.begin(); it!=names.end(); it++){
			std::cout<<"NAMES: "<<*it<<std::endl;
		}
	}
	return count;
}

/************************
 * Lua Helper Functions *
 ************************/

bool LuaBase::getField(const char* object, const char* method){
	bool foundObject = false;
	if (object) {
		lua_getglobal(L, object);  /* the component */
		if (lua_isnil(L, -1)){
			lua_pop(L, 1);
			std::cout<<"table \""<<object<<"\" not found"<<std::endl;
		}else{
			foundObject = true;
		}
	}
	
	if (foundObject) lua_getfield(L, -1, method);
	else lua_getfield(L, LUA_GLOBALSINDEX, method);
	
	return foundObject;
}

int LuaBase::callMethod(const char* object, const char* method){
		callMethod(object,method,std::vector<LuaArg*>(0));
}

template <typename Container>
int LuaBase::callMethod(const char* object, const char* method, Container args){
	bool isMethod = getField(object,method);
	if (lua_isfunction(L, -1) && (isMethod || !object)){
		if (isMethod) lua_getglobal(L, object);  /* "self" */
		for_each(args.begin(), args.end(), Pusher(L));
		switch (lua_pcall(L, args.size()+(isMethod?1:0), 0, 0)){
			case 0:
				break;
			case LUA_ERRRUN:
				std::cout<<"Runtime error while calling "<< method;
				if (isMethod) std::cout<<" from table "<<object;
				std::cout<< std::endl;
				break;
			case LUA_ERRMEM:
				std::cout<<"Memory error while calling "<< method;
				if (isMethod) std::cout<<" from table "<<object;
				std::cout<< std::endl;
				break;
			case LUA_ERRERR:
			default:
				std::cout<<"Error inception while calling  "<< method;
				if (isMethod) std::cout<<" from table "<<object;
				std::cout<< std::endl;
				break;
		}
	}else{
		std::cout<<"function "<<method<<" not found"<<std::endl;
	}
}

template <typename Container>
double LuaBase::callFunction(const char* function, Container &args) {
	callMethod(NULL,function,args);
}


/*********************
 * PROTECTED METHODS *
 *********************/

void LuaBase::writeWithType(G2DLUA_TYPE type){
	switch (type){
		case G2DINT:
			write<int>(lua_tostring(L, -2),lua_tonumber(L, -1));
			break;
			
		case G2DDOUBLE:
		default:
			write<double>(lua_tostring(L, -2),lua_tonumber(L, -1));
			break;
			
		case G2DFLOAT:
			write<float>(lua_tostring(L, -2),lua_tonumber(L, -1));
			break;
			
		case G2DSTRING:
			write<std::string>(lua_tostring(L, -2),std::string(lua_tostring(L, -1)));
			break;
			
		case G2DFUNCTION:
		case G2DBOOLEAN:
			write<bool>(lua_tostring(L, -2),lua_toboolean(L, -1));
			break;
			
	}
}

void LuaBase::readWithType(G2DLUA_TYPE type){
	const char* key = lua_tostring(L, -1);
	switch (type){
		case G2DINT:
			if (exists(key)) lua_pushinteger(L,read<int>(key));
			else lua_pushnil(L);
			break;
			
		case G2DDOUBLE:
		default:
			if (exists(key)) lua_pushnumber(L,read<double>(key));
			else lua_pushnil(L);
			break;
			
		case G2DFLOAT:
			if (exists(key)) lua_pushnumber(L,read<float>(key));
			else lua_pushnil(L);
			break;
			
		case G2DSTRING:
			if (exists(key)) lua_pushstring(L,read<std::string>(key).c_str());
			else lua_pushnil(L);
			break;
			
		case G2DBOOLEAN:
			if (exists(key)) lua_pushboolean(L,read<bool>(key));
			else lua_pushnil(L);
			break;
	}
}

void LuaBase::registerFunction(const char* key) {
	if (!exists(key)){
		write<float>(key,0.0f);
	}
	hook(key);
}

void LuaBase::unregisterFunction(const char* key) {
	//unhook(key);
}

LuaBase* LuaBase::getInstance(lua_State *L){
	lua_getglobal(L, THIS_AS_LIGHTUSERDATA);
	LuaBase *instance =	static_cast<LuaBase*>(lua_touserdata(L,-1));
	lua_pop(L,1);
	return instance;
}
int LuaBase::g2dlua_write(lua_State *L){
	int n = lua_gettop(L);    // number of arguments
	G2DLUA_TYPE type = G2DDOUBLE;
	switch (n){
		case 1://should be a table TODO: test
			if (!lua_istable(L, -1)) return 1;
			
			lua_pushstring(L, "name");
			lua_gettable(L, -2);  //table[name]
			if (lua_isnil(L, -1)) return 1;
			
			lua_pushstring(L, "value");
			lua_gettable(L, -3);  //table[value]
			if (lua_isnil(L, -1)) return 1;
			
			lua_pushstring(L, "type");
			lua_gettable(L, -4);  //table[type]
			if (lua_isnil(L, -1)) {
				lua_pop(L,1);
			}else{
				type = G2DLUA_TYPE(lua_tointeger(L, -1));
				lua_pop(L,1);
				break;
			}

		case 2://find out type
			if (lua_isnumber(L, -1)) {
				type = G2DDOUBLE;
			}else if (lua_isstring(L, -1)) {
				type = G2DSTRING;
			}else if (lua_isboolean(L, -1)) {
				type = G2DBOOLEAN;
			}
			std::cout<<"type: "<<type<<" of "<<(lua_tostring(L, -1))<<std::endl;
			break;
			
		default: //too many arguments
			lua_pop(L,n-3);
			
		case 3: //passing type as third argument
			if (lua_isnumber(L, -1)) {
				type = G2DLUA_TYPE(lua_tointeger(L, -1));
			}
			lua_pop(L,1);
			break;
	}
	
	getInstance(L)->writeWithType(type);
	
	return 0;
}

int LuaBase::g2dlua_read(lua_State *L){
	int n = lua_gettop(L);    //number of arguments
	G2DLUA_TYPE type = G2DDOUBLE;
	switch (n){
		case 1://should be a table
			if (lua_istable(L, -1)){
				lua_pushstring(L, "name");
				lua_gettable(L, -2);  //table[name]
				if (lua_isnil(L, -1)) return 1;
				
				lua_pushstring(L, "type");
				lua_gettable(L, -4);  // table[type]
				if (lua_isnil(L, -1)) {
					lua_pop(L,1);
				}else{
					type = G2DLUA_TYPE(lua_tointeger(L, -1));
					lua_pop(L,1);
					break;
				}
			}
			if (lua_isnumber(L, -1)) {
				type = G2DDOUBLE;
			}else if (lua_isstring(L, -1)) {
				type = G2DSTRING;
			}else if (lua_isboolean(L, -1)) {
				type = G2DBOOLEAN;
			}
			std::cout<<"type: "<<type<<" of "<<(lua_tostring(L, -1))<<std::endl;
			break;
			
		default: //too many arguments
			lua_pop(L,n-2);
			
		case 2: //passing type as argument
			if (lua_isnumber(L, -1)) {
				type = G2DLUA_TYPE(lua_tointeger(L, -1));
			}
			lua_pop(L,1);
			break;
	}
	
	getInstance(L)->readWithType(type);
	
	return 1;
}

int LuaBase::g2dlua_spawn(lua_State *L){
	getInstance(L)->spawn(lua_tostring(L,-1));
}

int LuaBase::g2dlua_registerFunction(lua_State *L){
	int n = lua_gettop(L);    //number of arguments
	switch (n){
		default: //too many arguments
			lua_pop(L,n-2);
		case 1:	
			getInstance(L)->registerFunction(lua_tostring(L,-1));
		case 0:
			break;
	}
	return 0;
}

int LuaBase::g2dlua_unregisterFunction(lua_State *L){
	int n = lua_gettop(L);    //number of arguments
	switch (n){
		default: //too many arguments
			lua_pop(L,n-2);
		case 1:
			getInstance(L)->unregisterFunction(lua_tostring(L,-1));
		case 0:
			break;
	}
}

void LuaBase::pushTypes(){
	lua_createtable(L,0,6);
	lua_pushinteger(L,G2DFLOAT);
	lua_setfield(L,-2,"float");
	lua_pushinteger(L,G2DDOUBLE);
	lua_setfield(L,-2,"double");
	lua_pushinteger(L,G2DINT);
	lua_setfield(L,-2,"int");
	lua_pushinteger(L,G2DBOOLEAN);
	lua_setfield(L,-2,"bool");
	lua_pushinteger(L,G2DSTRING);
	lua_setfield(L,-2,"string");
	lua_pushinteger(L,G2DFUNCTION);
	lua_setfield(L,-2,"func");
	lua_setglobal(L, "types");    /* types = table */
}

void LuaBase::registerStaticFunctions(){
	//TODO put in table "g2d" or in THIS_AS_LIGHTUSERDATA's __index
	lua_register(L,"g2d_read",g2dlua_read);
	lua_register(L,"g2d_write",g2dlua_write);
	lua_register(L,"g2d_spawn",g2dlua_spawn);
	lua_register(L,"g2d_registerFunction",g2dlua_registerFunction);
	lua_register(L,"g2d_unregisterFunction",g2dlua_unregisterFunction);
}

extern "C" {
	gear2d::component::base * build() {
		return new LuaBase();
	}
}

#ifdef LUA_BASE_CC_MAIN
int main(int argc, char **argv){
	LuaBase c;
	int n = 43;
	while (--n) c.update(n);
	return 0;
}
#endif
