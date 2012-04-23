#include <lua.hpp>
#ifndef LUAARG_H
#define LUAARG_H

enum G2DLUA_TYPE {G2DFLOAT, G2DDOUBLE, G2DINT, G2DBOOLEAN, G2DSTRING, G2DFUNCTION};

/**
 * Generic wrapper for arguments to be passed to lua
 */
class LuaArg{
	public:
		virtual void push(lua_State *L) const{
			lua_pushnil(L);
		}
	
	protected:
		
	private:
};

/**
 * Wrapper for numbers
 */
class LuaNumber : public LuaArg{
	public:
		template <typename T> 
		LuaNumber(T number):number(number){}
		
		virtual void push(lua_State *L) const {
			lua_pushnumber(L, number);
		}
		
	protected:
		double number;
	
	private:
		static LuaNumber popNumber(lua_State *L){
			double value;
			if (lua_isnumber(L, -1)) value = lua_tonumber(L, -1);
			else std::cout<<"Not a number"<<std::endl;
			return LuaNumber(value);
		}
};

/**
 * Wrapper for strings
 */
class LuaString : public LuaArg{
	public:
		template <typename T>
		LuaString(T str):str(str){};
		
		virtual void push(lua_State *L) const {
			lua_pushstring(L, str.c_str());
		}
		
	protected:
		std::string str;
	
	private:
		static LuaString popString(lua_State *L){
			std::string value;
			if (lua_isstring(L, -1)) value = std::string(lua_tostring(L, -1));
			else std::cout<<"Not a string."<<std::endl;
			return LuaString(value);
		}
};


/**
 * Functor class to push arguments into the lua state.
 */
class Pusher{
	public:
		Pusher(lua_State *L):L(L){};
		
		void operator()(const LuaArg *arg){
			arg->push(L);
		}
		
	protected:
		lua_State *L;
};

#endif
