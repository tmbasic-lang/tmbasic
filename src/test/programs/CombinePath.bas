#procedure
sub Main()
    dim x = CombinePath(["C:", "Users" + PathSeparator, "user", "Documents", "MyFile.txt"])
    if x = "C:" + PathSeparator + "Users" + PathSeparator + "user" + PathSeparator + "Documents" + PathSeparator + "MyFile.txt" then
        print "good"
    else
        print "bad: "; x
    end if
end sub
--output--
good
