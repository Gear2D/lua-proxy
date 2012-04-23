#ifndef LUABASE_H
#define LUABASE_H

#include <lua.hpp>
#include <stack>
#include <vector>
#include <set>
#include <string>
#include <cassert>
#include "gear2d.h"
#include "LuaArg.h"

/**
 * @file LuaBase.h
 * @author Igor Rafael de Sousa - igorrafaeldesousa@gmail.com
 * @brief This class provides a proxy for using components written in lua
 */

/**
 * @brief LuaBase is the base class for the proxy component.
 * 
 * This class reads may be extended if some specific behavior is desired
 * or to provide an specific file to be always loaded.
 */
class LuaBase : public gear2d::component::base {
	public:
		LuaBase(std::string name = std::string());
		~LuaBase();
		
		virtual gear2d::component::type type();
		virtual gear2d::component::family family();
		virtual std::string depends();
		virtual void handle(gear2d::parameterbase::id pid, base* lastwrite, gear2d::object::id owner);
		
		virtual void setup(gear2d::object::signature & si);
		std::list<std::string> loadFile(std::string name);
		virtual void update(gear2d::timediff);
		
		int parseComponentNames(std::string depends);
		
		static LuaBase *getInstance(lua_State *L);
		bool getField(const char* object, const char* method);
		void registerFunction(const char* key);
		void unregisterFunction(const char* key);
		int callMethod(const char* object, const char* method);
		template <typename Container>
		int callMethod(const char* object, const char* method, Container args);
		template <typename Container>
		double callFunction(const char* function, Container &args);
		
		
	protected:
		void writeWithType(G2DLUA_TYPE type);
		void readWithType(G2DLUA_TYPE type);
		static int g2dlua_read(lua_State *L);
		static int g2dlua_write(lua_State *L);
		static int g2dlua_spawn(lua_State *L);
		static int g2dlua_registerFunction(lua_State *L);
		static int g2dlua_unregisterFunction(lua_State *L);
		
		void pushTypes();
		void registerStaticFunctions();
	
		lua_State *L;
		std::list<std::string> names;
		std::list<std::string> components;
		
		static std::stack<LuaBase*> instances;

};

#endif