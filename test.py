import math


def bmi(x, y):
    bmi = x / (y ** 2)
    print("Your BMI is- " + str(bmi))


while True:
    intro = print("Hello! I'm Blue.")
    name = input("What's your name? ")
    print("Hello! " + name + ".")
    option = str(
        input(
            "What would you like me to find? BMI (or) BMR (or) BODY FAT PERCENTAGE? "
        ).lower()
    )
    x = input("Please type your weight in kilograms- ")
    y = input("Please type your height in metres- ")
    if option == "bmi":
        bmi(x, y)
    else:
        print("No such options :(")
