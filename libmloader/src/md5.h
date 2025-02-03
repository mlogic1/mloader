#ifndef MD5_H
#define MD5_H

#include <string>
#include <exception>

namespace mloader
{
	inline std::string CalculateGameMD5Hash(const std::string& releaseName)
	{
		// for now, this only works on linux and mac
		std::string result;

		char buffer[1024];
		snprintf(buffer, sizeof(buffer), "echo \"%s\" | md5sum | awk '{print $1}' | tr -d '\n'", releaseName.c_str());

		FILE* fp = popen(buffer, "r");
		if (fp == NULL)
		{
			perror("popen");
			throw std::runtime_error("Unable run MD5 hash");
		}
		
		char path[1035];
		// Read the output a line at a time - output it.
		while (fgets(path, sizeof(path), fp) != NULL) {
			result = path;
		}

		int status = pclose(fp);
		if (status == -1) 
		{
			perror("pclose");
			throw std::runtime_error("Unable run MD5 hash");
		}
		else 
		{
			// printf("Command exited with status: %d\n", WEXITSTATUS(status));
		}

		return result;
	}

}

#endif // MD5_H