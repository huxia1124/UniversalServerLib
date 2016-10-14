local inspect = require 'inspect'

--Global server object
print("----------------".."(timer tick = "..timerTick..")-------------------")
print("Receive: "..math.floor((server.ReceivedBytesPerSecond/1024)).." KB/s,\tPackages: "..server.ReceivedCountPerSecond.." pkg/sec,\t Current clients: "..server:GetTcpClientCount())
print("         "..math.floor((server.TotalReceivedBytes/1024/1024)).." MB,\tPackages: "..server.TotalReceivedCount.." pkg")
print("Send:    "..math.floor((server.SentBytesPerSecond/1024)).." KB/s,\tPackages: "..server.SentCountPerSecond.." pkg/sec")
print("         "..math.floor((server.TotalSentBytes/1024/1024)).." MB,\tPackages: "..server.TotalSentCount.." pkg")

--Set timer interval for the server
server.TimerInterval = 30000

--using global key value storage
_data.put("key1", "value1")
_data.put("key2", "value2")
print(_data.get("key1"))

--Using global shared data object

a = SharedData()

a:RegisterStringVectorVariable("Test")
a:AddStringValue("Test", "T1")
a:AddStringValue("Test", "T2")
a:AddStringValue("Test", "T3")
a:AddStringValue("Test", "T4")

a:RegisterIntegerVariable("Test2\\Test-2-1")
a:SetIntegerValue("Test2\\Test-2-1", 123)

a:RegisterStringSetVariable("Test2\\Test-2-2")
a:AddStringValue("Test2\\Test-2-2", "abc")
a:AddStringValue("Test2\\Test-2-2", "bcd")
a:AddStringValue("Test2\\Test-2-2", "abc")	--this duplicated value will be ignored by the set

print("------------------------------------")
function display(node, level)
  for x in node:GetNodes() do
  print(string.rep(" ", level*2)..x.Name)
  local n = x:GetValues()
  for y in n do
  print(string.rep(" ", level*2+1)..tostring(inspect(y)))
  end
  display(x, level+1)
  end
end

print(a.Name)
display(a, 1)