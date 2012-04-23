g2dbuffer = {}

if not types then
	types = {int = 1, float = 2, double = 3, string = 4}
end

function g2dread(table, key, g2dstring)
	local export = rawget(table, "__g2d_export")
	if export[key] then
		if type(export[key])=="table" then
			--TODO store keys recursively
		else
			g2dbuffer[key] = g2d_read(key, export[key])
			return g2dbuffer[key]
		end
	end
	if rawget(table,key) then
		return rawget(table,key)
	elseif rawget(table,"__original_mt") then --TODO test
		local altindex = rawget(table,"__original_mt").__index
		if altindex then
			if type(altindex)=="function" then
				return altindex(key)
			else
				return altindex[key]
			end
		end
	end
end

function g2dwrite(table, key, value)
	local export = rawget(table, "__g2d_export")
	if export and export[key] then
		if type(export[key])=="table" then
			--TODO
		else
			g2dbuffer[key] = value
			g2d_write(key,value,export[key])
		end
	elseif rawget(table,"__original_mt") then --TODO test
		local altnewindex = rawget(table,"__original_mt").__newindex
		if altnewindex then
			if type(altnewindex)=="function" then
				altnewindex(table, key, value)
			else
				altnewindex[key] = value
			end
		end
	else
		rawset(table, key, value)
	end
end

__g2d_mt = 
	{
		__index = g2dread,
		__newindex = g2dwrite,
	}


--TODO move to C++
function g2dregister(self, key, value, init)
	if type(value)=="function" then
		rawget(self, "__g2d_handlers")[key] = value
		g2d_registerFunction(key)
		--rawset(self, key, function()self[key]=1 end
	else
		self.__g2d_export = self.__g2d_export or {}
		
		self.__g2d_export[key] = value
		if init then
			self[key] = init
		end
	end
end
