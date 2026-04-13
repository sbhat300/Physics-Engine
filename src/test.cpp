#include <unordered_map>
#include <iostream>

int main()
{
    std::unordered_map<int, int> test;
    test[1] = 2;
    test[2] = 2;
    test.erase(1);
    std::cout << "start" << std::endl;
    for(std::pair<int, int> obj : test)
    {
        std::cout << obj.first << " " << obj.second << std::endl;
    } 
}