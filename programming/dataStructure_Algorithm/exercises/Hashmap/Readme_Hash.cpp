Applications of Hash-map
The key here is under what scenarios should I natually come up with the idea of using Hash map. The following are some 

//Part I Using unordered_map to achieve constant search.
//Unordered map is an associative container that contains key - value pairs with UNIQUE keys.Search, insertion, and removal of elements have average constant - time complexity.
std::unordered_map<std::string, int> map;
//A few ways to populate an map
//Way 1:
map.insert({ "key1", 2 }); //Note I need {} for uniform initialization. 
map.insert({ "key2", 3 });

//Way2:
map.insert(make_pair("key1", 2)); //If I don't use {} as above, then I can use make_pair() to populate a map.
map.insert(make_pair("key2", 3));

//Way3:
map["key1"] = 2; //This way is just like Python. So memorizing this one I can handle both c++ and Python.
map["key2"] = 3;

auto it = map.find("key1");
if (it != map.end())
std::cout << "The value for key1 is " << it->second << std::endl;


