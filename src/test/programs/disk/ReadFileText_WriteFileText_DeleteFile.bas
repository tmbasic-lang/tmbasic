sub Main()
    ' deleting a file that already doesn't exist is not an error
    DeleteFile "this_file_does_not_exist"
    DeleteFile "this_file_does_not_exist"

    ' round trip a file
    DeleteFile "tmbasic-test"
    WriteFileText "tmbasic-test", "Hello, World!"
    print ReadFileText("tmbasic-test")
    DeleteFile "tmbasic-test"

    ' ReadFileText should throw an error if the file doesn't exist
    try
        print ReadFileText("this_file_does_not_exist")
    catch
        if ErrorCode() = ERR_FILE_NOT_FOUND then
            print "1 ok"
        else
            print "1 "; ErrorCode(); " "; ErrorMessage()
        end if
    end try

    ' ReadFileText should throw an error if the directory doesn't exist
    try
        print ReadFileText("/this_directory_does_not_exist/this_file_does_not_exist")
    catch
        if ErrorCode() = ERR_FILE_NOT_FOUND then
            print "2 ok"
        else
            print "2 "; ErrorCode(); " "; ErrorMessage()
        end if
    end try

    ' WriteFileText should throw an error if the directory doesn't exist
    try
        WriteFileText "/missing/dir/tmbasic-test", "asdf"
    catch
        if ErrorCode() = ERR_FILE_NOT_FOUND then
            print "3 ok"
        else
            print "3 "; ErrorCode(); " "; ErrorMessage()
        end if
    end try

    ' WriteFileText should throw an error if the filename is actually a directory
    CreateDirectory "testdir"
    try
        WriteFileText "testdir", "asdf"
    catch
        if ErrorCode() = ERR_PATH_IS_DIRECTORY then
            print "4 ok"
        else if ErrorCode() = ERR_ACCESS_DENIED then
            ' on Windows we get access denied
            print "4 ok"
        else
            print "4 "; ErrorCode(); " "; ErrorMessage()
        end if
    end try
    DeleteDirectory "testdir"
end sub
--output--
Hello, World!
1 ok
2 ok
3 ok
4 ok
