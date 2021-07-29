#procedure
sub Main()
    try
        throw "AAA"
    catch
        throw "BBB"
    end try
end sub
--output--
Error
0
BBB
