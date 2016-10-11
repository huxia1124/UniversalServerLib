
	err, ret = xpcall(function()

		print("----------------Worker Thread Initialization -------------------")
		
		myThreadData = myThreadData or {}
		
	end, debug.traceback)
	
	if (ret) then
		print("complicated(true) --> ", err, ret)
		--server:Log(err)   
		end



