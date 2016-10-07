
	err, ret = xpcall(function()
	
		 print("收到数据!! : TID = "..utils.GetThreadId())
		 local msg = utils.GetMessage()
		 local firstString = msg:GetNextString()
		 print(firstString)
		 server:Log(firstString)  
	end, debug.traceback)
	
	if (ret) then
		print("complicated(true) --> ", err, ret)
		--server:Log(err)   
		end



