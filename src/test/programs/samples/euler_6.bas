sub Main()
    dim sumOfSquares = 0
    dim sum = 0

    for i = 1 to 100
        sumOfSquares = sumOfSquares + i * i
        sum = sum + i
    next

    dim squareOfSum = sum * sum
    dim difference = squareOfSum - sumOfSquares

    print difference
end sub

--output--
25164150
