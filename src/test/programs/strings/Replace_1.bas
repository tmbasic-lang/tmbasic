#procedure
sub Main()
    print "1:"; Replace("aaa", "a", "bb")
    print "2:"; Replace("aaa", "aa", "bb")
    print "3:"; Replace("", "a", "b")
    print "4:"; Replace("aaa", "a", "aa")
    print "5:"; Replace("aaaa", "aa", "a")
end sub

--output--
1:bbbbbb
2:bba
3:
4:aaaaaa
5:aa
