#procedure
sub Main()
    dim path = "dir-test" + PathSeparator()
    try
        DeleteDirectory path, true
    catch
        ' it's ok, just cleaning up
    end try

    CreateDirectory path
    WriteFileText PathCombine([path, "test.txt"]), "1 output from dir-test/test.txt"
    print ReadFileText(PathCombine([path, "test.txt"]))

    ' DeleteDirectory(String) with non-empty dir -> throws
    try
        DeleteDirectory path
        print "bad"
    catch
        if ErrorCode() = ERR_DIRECTORY_NOT_EMPTY then
            print "2 ERR_DIRECTORY_NOT_EMPTY"
        else
            print "2 "; ErrorCode(); " "; ErrorMessage()
        end if
    end try

    ' DeleteDirectory(String, false) with non-empty dir -> throws
    try
        DeleteDirectory path, false
        print "bad"
    catch
        if ErrorCode() = ERR_DIRECTORY_NOT_EMPTY then
            print "3 ERR_DIRECTORY_NOT_EMPTY"
        else
            print "3 "; ErrorCode(); " "; ErrorMessage()
        end if
    end try

    ' DeleteDirectory(String, true) with non-empty dir -> works
    print "3.5 cleaning up"
    DeleteDirectory path, true
    print "3.5 cleanup done"

    ' confirm that the directory is gone now
    try
        WriteFileText PathCombine([path, "test.txt"]), "asdf"
        print "bad"
    catch
        if ErrorCode() = ERR_FILE_NOT_FOUND then
            print "4 ERR_FILE_NOT_FOUND"
        else
            print "4 "; ErrorCode(); " "; ErrorMessage()
        end if
    end try

    ' test ListDirectories and ListFiles
    CreateDirectory PathCombine(["testdir", "subdir1"])
    CreateDirectory PathCombine(["testdir", "subdir2"])
    WriteFileText PathCombine(["testdir", "subfile1"]), "111"
    WriteFileText PathCombine(["testdir", "subfile2"]), "222"
    WriteFileText PathCombine(["testdir", "subfile3"]), "333"
    print "5 "; Len(ListDirectories("testdir"))
    print "6 "; Len(ListFiles("testdir"))
    DeleteDirectory "testdir", true
end sub
--output--
1 output from dir-test/test.txt
2 ERR_DIRECTORY_NOT_EMPTY
3 ERR_DIRECTORY_NOT_EMPTY
3.5 cleaning up
3.5 cleanup done
4 ERR_FILE_NOT_FOUND
5 2
6 3
