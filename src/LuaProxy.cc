#include <iostream>
#include "LuaProxy.h"

using namespace gear2d;
using namespace gear2d::component;

LuaProxy::LuaProxy(lua_State *L, std::string family, std::string type)
	: L(L), family_(family), type_(type){
		//Execute loaded file (properly place tables in memory)
		lua_pcall(L, 0, 0, 0);
		
		//Get the main table of this file (the component)
		lua_getglobal(L, family.c_str());
		
		//ensures existence of field G2D_LUA_INTERFACE_TABLE
		lua_getfield(L, -1, G2D_LUA_INTERFACE_TABLE);
		if (lua_isnil(L,-1)){
			lua_createtable(L,0,0);
			lua_setfield(L,-3,G2D_LUA_INTERFACE_TABLE);
			lua_pop(L,1);
		}

		//ensures existence of field G2D_LUA_HANDLER_TABLE
		//as a field in the main table of this file (the component)
		lua_getfield(L, -1, G2D_LUA_HANDLER_TABLE);
		if (lua_isnil(L,-1)){
			lua_createtable(L,0,0);
			lua_setfield(L,-3,G2D_LUA_HANDLER_TABLE);
		}
		lua_pop(L,1);
		
		//conditionally forward indexing of the component to gear2d
		if(lua_getmetatable(L,-1)){//try to store the current metatable
			lua_getglobal(L, G2D_LUA_METATABLE);
			lua_setmetatable(L,-3);
			lua_setfield(L, -1, G2D_LUA_ORIGINAL_METATABLE);
		}else{
			lua_getglobal(L, G2D_LUA_METATABLE);
			lua_setmetatable(L,-2);
		}
		
		//function for registering variables and handlers
		lua_getglobal(L, "g2dregister");
		lua_setfield(L, -2, "register");
		
}
	
LuaProxy::~LuaProxy(){
	//TODO remove table from lua environment
}
	
component::type LuaProxy::type() { return type_; }
component::family LuaProxy::family() { return family_; }
std::string LuaProxy::depends() { return ""; } //TODO let g2d handle dependencies
		
void LuaProxy::setup(object::signature & si) {
	
}
		
void LuaProxy::update(timediff dt) {
	
}
