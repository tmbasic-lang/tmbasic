#procedure
sub Main()
    dim raceTimes = [47, 98, 66, 98]
    dim raceRecords = [400, 1213, 1011, 1540]
    dim totalWays = 1

    for i = 0 to Len(raceTimes) - 1
        dim waysToWin = CountWaysToWin(raceTimes(i), raceRecords(i))
        totalWays = totalWays * waysToWin
    next

    print totalWays
end sub

#procedure
function CountWaysToWin(time as Number, rec as Number) as Number
    dim count = 0
    for holdTime = 0 to time - 1
        dim distance = (time - holdTime) * holdTime
        if distance > rec then
            count = count + 1
        end if
    next
    return count
end function

--output--
1660968
