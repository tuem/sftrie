/*
sftrie
https://github.com/tuem/sftrie

Copyright 2017 Takashi Uemura

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "history.hpp"

#include <iomanip>

History::History()
{
	time_records.push_back({std::chrono::system_clock::now(), {}, -1});
}

void History::record(const std::string& task, int count)
{
	time_records.push_back({std::chrono::system_clock::now(), task, count});
}

void History::refresh()
{
	time_records.push_back({std::chrono::system_clock::now(), {}, -1});
}

void History::dump(std::ostream& os) const
{
	os <<
		std::right << std::setw(26) << "task" <<
		std::right << std::setw(12) << "time[us]" <<
		std::right << std::setw(12) << "count" <<
		std::right << std::setw(16) << "average[ns]" <<
		std::endl;
	for(size_t i = 1; i < time_records.size(); ++i){
		if(time_records[i].count < 0)
			continue;
		auto t = std::chrono::duration_cast<std::chrono::microseconds>(
				time_records[i].time - time_records[i - 1].time).count();
		os <<
			std::right << std::setw(26) << time_records[i].task <<
			std::right << std::setw(12) << std::setprecision(8) << t;
		if(time_records[i].count > 1)
			os <<
				std::right << std::setw(12) << time_records[i].count <<
				std::right << std::setw(16) << std::setprecision(12) <<
					(static_cast<double>(t * 1000) / time_records[i].count);
		os << std::endl;
	}
}
