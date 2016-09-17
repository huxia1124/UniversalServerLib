 require "recv_add"
local inspect = require 'inspect'
   
	err, ret = xpcall(function()
	
	local uid = utils.GetClientUid()
	local mm = utils.GetMessage()

	local m1 = STXProtocol();
	local st1 = mm.NextFieldTypeString  
	local s1 = mm:GetNextString()
	local f1 = mm:GetNextFloat()
	--t.uid = uid..""
	ss = server:GetString("k1")
		 print(ss.." : TID = "..utils.GetThreadId())
		 print(s1.." : "..st1)
		 print(f1)
		 m1:AppendByte(1)
		 m1:AppendWord(2)
		 m1:AppendDWord(4)
		 m1:AppendI64(8)
		 m1:AppendString("sssss")
		 
		 --server:Log(tostring(inspect(package.loaded)))
	 
	 --local b = false
	 --server:Log(tostring(collectgarbage("count")))
		--server:SendStringToClient(uid, domore()..domore2())
		server:SendPackageToClient(uid, m1)
		--collectgarbage("collect")

		--server = nil
	end, debug.traceback)
	
	if (ret) then
		print("complicated(true) --> ", err, ret)
		--server:Log(err)
		end



