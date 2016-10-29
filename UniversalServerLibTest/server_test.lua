
function start()

	local err, ret = xpcall(function()

	--local sss=utils.GetServer()

		--server:SetRPCPort(3399)
		server.TimerInterval = 1000   --Default value is 60000
		server:SetTimerScript("scripts/timer.lua")
		server:SetFileChangedScript("scripts/file_changed.lua")
		server:SetWorkerThreadInitializationScript("scripts/worker_thread_initialization.lua")
		server:SetTcpServerClientConnectedScript(6800, "scripts/new_client.lua")
		server:SetTcpServerClientDisconnectedScript(6800, "scripts/client_disconnect.lua")
		server:SetUdpServerReceivedScript(9200, "scripts/udp_recv_test.lua")

		server:StartRPC(3399)
		server:StartServer(0, 20000, 50000, 2048)
		utils.Sleep(300)
		server:SetTcpServerReceivedScript(6800, "scripts/recv_test.lua")
		server:SetTcpServerReceivedScript(9000, "scripts/recv_var.lua")
		server:BeginTcpStreamServer(6800, "tcpStream", 100)
		server:BeginUdpServer(9200, "udp")
		--local n = server:CreateTcpStreamConnection("127.0.0.1", 16800)
		--local n2 = server:PendingTcpStreamConnection("127.0.0.1", 16800)
		--print("Connection ID = "..n.." and "..n2)
		--server:SendStringToConnection(n, "aaaaaaaaaa")
		
		server:BeginHttpServer(8090, "http", 10)
		server:BeginTcpServerV(9000, "tcpV", 10)
		
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

