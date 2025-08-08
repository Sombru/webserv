#include <iostream>
#include <string>


class Robot {
    private:
        std::string name;
        int batteryLevel;
    public:
        Robot(std::string name); // constructor
        Robot(Robot const &scopy); // copy constructor
        Robot &operator=(Robot const &src); // assignment
        ~Robot(); // destructor

        void sayHello() const; 
        void charge(); // sets baterry to 100
};
Robot* createRobot(std::string name); // REturns a new heap-allocated Robot
void deployRobot(std::string name); // Creates a stack-allocated Robot and calls sayHello()

int main(void) {
    Robot* r1 = createRobot("Alpha");
    r1->sayHello();

    deployRobot("Bravo");

    Robot r2("Charlie");
    r2.sayHello();

    Robot r3 = r2; // Test copy constructor
    r3.charge();

    r3 = *r1; // Test assignment operator

    delete r1; // Clean up
    return 0;
}

Robot::Robot(std::string name) {
    this->name = name;
    std::cout << this->name << " was created\n";
}

Robot::Robot(Robot const &copy) {
    std::cout << "Copy constructor called for " << copy.name << std::endl;
    this->name = copy.name;
    this->batteryLevel = copy.batteryLevel;
}

Robot &Robot::operator=(Robot const &copy) {
    if (this != &copy) {
        this->name = copy.name;
        this->batteryLevel = copy.batteryLevel;
    }
    return (*this);
}

Robot::~Robot() {
    std::cout << "Destructor called\n";
}

void Robot::sayHello() const {
    std::cout << "Hello\n";
}

void Robot::charge() {
    this->batteryLevel = 100;
}

Robot* createRobot(std::string name)  // REturns a new heap-allocated Robot
{
    Robot* zuzu = new Robot(name);
    return (zuzu);
}

void deployRobot(std::string name) // Creates a stack-allocated Robot and calls sayHello()
{
    Robot r1(name);
    r1.sayHello();
}