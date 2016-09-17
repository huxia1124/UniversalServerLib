
function start()


	local err, ret = xpcall(function()

	--local sss=utils.GetServer()
		server:StartServer(0, 10000, 10000, 2048)
		utils.Sleep(300)
		server:BeginTcpStreamServer(6800, "tcpStream", 10000)
		local n = server:CreateTcpStreamConnection("127.0.0.1", 16800)
		local n2 = server:PendingTcpStreamConnection("127.0.0.1", 16800)
		print("Connection ID = "..n.." and "..n2)
		server:SendStringToConnection(n, "aaaaaaaaaa")
		
		--server:BeginHttpServer(8080, "http", 10)
		--server:BeginTcpServerV(9000, "tcpV", 10)
		
		server:PutString("k1", "Abcd")
		
	end, debug.traceback)
	if(ret) then
	print("complicated(true) --> ", err, ret)
	end
end


waitTime = 30000
function stop()

print("stop called!") 

	local err, ret = xpcall(function()
	server:Terminate()
	end, debug.traceback)
	
	if(ret) then
	print("complicated(true) --> ", err, ret)
	end
end

function show_stop_time()

	local err, ret = xpcall(function()
	local waitSec = waitTime / 1000
	if(waitSec == 0)then
		print("-------------------Server will never terminate automatically--------------------");
	else
		print("-------------------Server will be shutdown in "..waitSec.." seconds--------------------");
	end
	end, debug.traceback)
	
	if(ret) then
	print("complicated(true) --> ", err, ret)
	end
end
