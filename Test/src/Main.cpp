#include <yaml-cpp/yaml.h>
#include <Filesystem/Filesystem.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
	Core::Filesystem filesystem(argv[0]);
	std::string pathToYamlFile = filesystem.GetAbsolutePath("../../testfile.yml");
	YAML::Node data = YAML::LoadFile(pathToYamlFile);

	if (!data["Employees"])
	{
		std::cout << "error" << std::endl;
	}

	auto employees = data["Employees"];
	
	for (int i = 0; i < employees.size(); ++i)
	{
		std::cout << employees[i]["name"]  << " is good at " << employees[i]["skills"][0] << std::endl;
	}

	return 0;
}