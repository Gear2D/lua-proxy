#if 0
#!/bin/bash
g++ teste.cc -I/usr/local/include/gear2d -fPIC -shared -Wl,-soname,libteste.so -o libteste.so -lgear2d
exit
#endif

#include <iostream>
#include <string>
#include "gear2d.h"
#include <lua.hpp>


using namespace gear2d;
using namespace gear2d::component;


namespace 
{
    const char* G2D_LUA_HEADER = "g2dcomponent.lua";
    const char* G2D_LUA_METATABLE = "__g2d_mt";
    const char* G2D_LUA_ORIGINAL_METATABLE = "__original_mt";
    const char* G2D_LUA_HANDLER_TABLE = "__g2d_handlers";
    const char* G2D_LUA_INTERFACE_TABLE = "__g2d_export";
    const char* THIS_AS_LIGHTUSERDATA = "__g2dproxy";
    const char* LUA_COMPONENTS_SIGNATURE_KEY = "luattach";
	
	/* TODO: redo. */
    const char* G2D_COMPONENT = 
"g2dbuffer = {}\n"
"\n"
"if not types then\n"
"	types = {int = 1, float = 2, double = 3, string = 4}\n"
"end\n"
"\n"
"function g2dread(table, key, g2dstring)\n"
"	local export = rawget(table, \"__g2d_export\")\n"
"	if export[key] then\n"
"		if type(export[key])==\"table\" then\n"
"			--TODO store keys recursively\n"
"		else\n"
"			g2dbuffer[key] = g2d_read(key, export[key])\n"
"			return g2dbuffer[key]\n"
"		end\n"
"	end\n"
"	if rawget(table,key) then\n"
"		return rawget(table,key)\n"
"	elseif rawget(table,\"__original_mt\") then --TODO test\n"
"		local altindex = rawget(table,\"__original_mt\").__index\n"
"		if altindex then\n"
"			if type(altindex)==\"function\" then\n"
"				return altindex(key)\n"
"			else\n"
"				return altindex[key]\n"
"			end\n"
"		end\n"
"	end\n"
"end\n"
"\n"
"function g2dwrite(table, key, value)\n"
"	local export = rawget(table, \"__g2d_export\")\n"
"	if export and export[key] then\n"
"		if type(export[key])==\"table\" then\n"
"			--TODO\n"
"		else\n"
"			g2dbuffer[key] = value\n"
"			g2d_write(key,value,export[key])\n"
"		end\n"
"	elseif rawget(table,\"__original_mt\") then --TODO test\n"
"		local altnewindex = rawget(table,\"__original_mt\").__newindex\n"
"		if altnewindex then\n"
"			if type(altnewindex)==\"function\" then\n"
"				altnewindex(table, key, value)\n"
"			else\n"
"				altnewindex[key] = value\n"
"			end\n"
"		end\n"
"	else\n"
"		rawset(table, key, value)\n"
"	end\n"
"end\n"
"\n"
"__g2d_mt = \n"
"	{\n"
"		__index = g2dread,\n"
"		__newindex = g2dwrite,\n"
"	}\n"
"\n"
"\n"
"--TODO move to C++\n"
"function g2dregister(self, key, value, init)\n"
"	if type(value)==\"function\" then\n"
"		rawget(self, \"__g2d_handlers\")[key] = value\n"
"		g2d_registerFunction(key)\n"
"		--rawset(self, key, function()self[key]=1 end\n"
"	else\n"
"		self.__g2d_export = self.__g2d_export or {}\n"
"		\n"
"		self.__g2d_export[key] = value\n"
"		if init then\n"
"			self[key] = init\n"
"		end\n"
"	end\n"
"end\n"
"";
}


class LuaProxy : public gear2d::component::base {
	public:
		LuaProxy(lua_State *L, std::string family, std::string type);
	
		~LuaProxy();
	
		virtual component::type type();
		virtual component::family family();
		virtual std::string depends();
		
		virtual void setup(object::signature & si);
		
		virtual void update(timediff dt);
		
	private:
		lua_State *L;
		std::string family_, type_;
};
