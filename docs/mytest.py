# Simple Python Script

def greet(name):
    """Print a greeting message."""
    print(f"Hello, {name}!")

class Person:
    def __init__(self, name, age):
        self.name = name
        self.age = age

    def display_info(self):
        print(f"Name: {self.name}, Age: {self.age}")

if __name__ == "__main__":
    p = Person("Alice", 30)
    p.display_info()
    greet("Bob")

# Sample loop and condition
for i in range(5):
    if i % 2 == 0:
        print(f"{i} is even")
    else:
        print(f"{i} is odd")
