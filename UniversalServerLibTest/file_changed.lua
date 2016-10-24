
	err, ret = xpcall(function()

		   local action = utils.GetFileChangeAction()
		   local actionString = utils.GetFileChangeActionString()
		   local path = utils.GetChangedFileRelativePathName()
		   local fullpath = utils.GetChangedFileFullPathName()

		   print(path.."   "..actionString.."["..action.."]")
		   print("\tFullPath:  "..fullpath)
		
	end, debug.traceback)
	
	if (ret) then
		print("complicated(true) --> ", err, ret)
		--server:Log(err)   
		end



