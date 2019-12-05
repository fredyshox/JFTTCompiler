#!/usr/bin/env python3
# Compares 5 number generation techniques

import matplotlib.pyplot as plt

def method1(value):
    cost = 10
    if value == 0:
        return cost

    value = abs(value)
    if (value % 2 != 0):
        cost += 1
        value -= 1
    
    cost += value // 2
    value = value // 2
    if (value < 10):
        cost += value
    else:
        cost += 10

    return cost

def method1_shift(value, shiftCost=True):
    cost = 10
    if value == 0:
        return cost

    value = abs(value)
    if (value % 2 != 0):
        cost += 1
        value -= 1
    
    if shiftCost:
        cost += 11
    cost += value // 2
    value = value // 2
    if (value < 5):
        cost += value
    else:
        cost += 5

    return cost

def method2(value):
    cost = 10
    if value == 0:
        return cost

    value = abs(value)
    if (value % 2 != 0):
        cost += 1
        value -= 1

    while (value & 1) == 0 and value > 10:
        cost += 10
        value = value >> 1

    cost += value
    return cost

def method2_shift(value, shiftCost=True):
    cost = 10
    if value == 0:
        return cost

    value = abs(value)
    if (value % 2 != 0):
        cost += 1
        value -= 1
    
    if shiftCost:
        cost += 11
    while (value & 1) == 0 and value > 5:
        cost += 5
        value = value >> 1

    cost += value
    return cost

def method3(value):
    cost = 10
    if value == 0:
        return cost

    if value > 0:
        cost += 12
    else:
        cost += 12

    value = abs(value)
    while value != 0:
        cost += 10
        value = value >> 1

        if value != 0:
            cost += 45

    return cost

if __name__=='__main__':
    x = list(range(0, 2000))
    m1  = map(lambda x: method1(x), x)
    m1s = map(lambda x: method1_shift(x, False), x)
    m2  = map(lambda x: method2(x), x)
    m2s = map(lambda x: method2_shift(x, False), x)
    m3 = map(lambda x: method3(x), x)

    plt.plot(x, list(m1), '-b')
    plt.plot(x, list(m1s), '-r')
    plt.plot(x, list(m2), '-g')
    plt.plot(x, list(m2s), '-m')
    plt.plot(x, list(m3), '-y')
    print("Done! Now Drawing...")
    plt.show()

