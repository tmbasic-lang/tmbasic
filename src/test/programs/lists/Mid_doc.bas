sub Main()
    dim myList = [1, 2, 3, 4, 5]
    dim newList = Mid(myList, 1, 3)
    for each element in newList
        print element
    next
end sub
--output--
2
3
4
