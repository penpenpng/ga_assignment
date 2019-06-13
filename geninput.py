import math


def gen_sqrts(size):
    for i in range(1, size + 1):
        yield f"{str(math.sqrt(i) / math.sqrt(size))[:30]}\n"


with open("data/input_sqrt_100", "w") as f:
    f.writelines(gen_sqrts(100))


with open("data/input_sqrt_500", "w") as f:
    f.writelines(gen_sqrts(500))


with open("data/input_sqrt_1000", "w") as f:
    f.writelines(gen_sqrts(1000))
