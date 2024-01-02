sub Main()
    dim myList = ListFill("Hi!", 3)
    print Len(myList)
    for each element in myList
        print element
    next
end sub
--output--
3
Hi!
Hi!
Hi!
