#!/usr/bin/env python3

import math
import random

import numpy             as np
import matplotlib.pyplot as plt

from scipy.optimize import curve_fit

def p(n, d):
    '''Return the probability that the first
    active scan is performed on the n-th attempt.

    Arguments:
    n -- number of the current attempt
    d -- (r-1)/d is the probability that an active scan is performed on attempt r
    '''
    if n == 1:
        return 0

    product = 1.0
    for r in range(1, n):
        product *= (1.0 - (r - 1.0) / d)

    product *= (n - 1.0) / d
    return product

def wp(n, d):
    '''Returns the number of attempts weighted by their probability
    that first active scan is performed on the n-th attempt.

    Arguments:
    n -- number of the current attempt
    d -- (r-1)/d is the probability that an active scan is performed on attempt r
    '''
    return n * p(n, d)

def expected(d):
    '''Returns the approximated expected number of attempts given d

    Arguments:
    d -- (r-1)/d is the probability that an active scan is performed on attempt r
    '''
    x = range(1, 500)
    y = [wp(n, d) for n in x]
    return sum(y)

def experiment(d):
    '''Returns the number of attempts required in a single simulation'''
    count = 0
    while(True):
        r = random.randint(0, d-1)
        if count > r:
            return count + 1
        count += 1

def repeated_experiment(d):
    values = [experiment(d) for i in range(0, 100 + d)]
    return sum(values) / len(values)

def hypothesis(d):
    return 0.6729 + math.sqrt(math.pi / 2.0 * d)

def func(x, a):
    return a + np.sqrt(math.pi / 2.0 * x)

def main():
    # plot selection of curves
    x = range(1, 200) # values for n
    plt.plot(x, [wp(n, 32) for n in x],  label='32')
    plt.plot(x, [wp(n, 64) for n in x],  label='64')
    plt.plot(x, [wp(n, 128) for n in x], label='128')
    plt.plot(x, [wp(n, 256) for n in x], label='256')
    plt.legend()
    plt.show()

    # plot expected number of attempts
    x = range(1, 500, 10) # values for d
    plt.plot(x, [expected(d) for d in x], label='expected')
    plt.plot(x, [repeated_experiment(d) for d in x], label='experiment')
    plt.plot(x, [hypothesis(d) for d in x], label='hypothesis')
    plt.xlabel('x')
    plt.ylabel('y')
    plt.legend()
    plt.show()

    # optimize curve
    x = range(1, 1000)
    y = np.array([expected(d) for d in x])
    x = np.array(x)

    popt, pcov = curve_fit(func, x, y)
    print(popt)
    print(pcov)

    plt.plot(x, y, 'b-', label='data')
    plt.plot(x, func(x, *popt), 'r-', label='fit')
    plt.xlabel('x')
    plt.ylabel('y')
    plt.legend()
    plt.show()

if __name__ == '__main__':
    main()
