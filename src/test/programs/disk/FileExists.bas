#procedure
sub Main()
    DeleteFile "myfile.txt"
    print FileExists("myfile.txt")
    WriteFileText "myfile.txt", "Hello"
    print FileExists("myfile.txt")
    DeleteFile "myfile.txt"
    print FileExists("myfile.txt")
end sub
--output--
false
true
false
