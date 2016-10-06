
function start()

	local err, ret = xpcall(function()

	--local sss=utils.GetServer()
		server:StartServer(0, 10000, 10000, 2048)
		server:SetTcpServerReceiveScript(6800, "recv_stream.lua");		--Associate the script to be executed when tcp server received data from client
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

function stop()

print("stop called!") 

	local err, ret = xpcall(function()
	server:Terminate()
	end, debug.traceback)
	
	if(ret) then
	print("complicated(true) --> ", err, ret)
	end
end
