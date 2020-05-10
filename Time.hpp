#include <Header.h>

namespace ct
{
	const std::map<int, QString> MONTHS{
		{ 1, "Январь" },
		{ 2, "Февряль" },
		{ 3, "Март" },
		{ 4, "Апрель" },
		{ 5, "Май" },
		{ 6, "Июнь" },
		{ 7, "Июль" },
		{ 8, "Август" },
		{ 9, "Сентябрь" },
		{ 10, "Октябрь" },
		{ 11, "Ноябрь" },
		{ 12, "Декабрь" }
	};

	const std::map<int, QString> WEEKS{
		{ 0, "Воскресенье" },
		{ 1, "Понедельник" },
		{ 2, "Вторник" },
		{ 3, "Среда" },
		{ 4, "Четверг" },
		{ 5, "Пятница" },
		{ 6, "Суббота" }
	};

	enum class type : int
	{
		last_access,
		last_write,
		create
	};

	QString from_filetime(const u64& data)
	{
		FILETIME ft;
		ft.dwLowDateTime = data & 0xFFFFFFFF;
		ft.dwHighDateTime = (data & 0xFFFFFFFF00000000) >> 32;
		SYSTEMTIME st;
		FileTimeToSystemTime(&ft, &st);
		return
			QString{
			QString::number(st.wDay) + "." +
			QString::number(st.wMonth) + "." +
			QString::number(st.wYear) + " " +
			WEEKS.at(st.wDayOfWeek) + " " +
			QString::number(st.wHour + 3) + ":" +
			QString::number(st.wMinute) + ":" +
			QString::number(st.wSecond) + "." +
			QString::number(st.wMilliseconds)
		};
	}
	QString from_filetime(const FILETIME& data)
	{
		SYSTEMTIME st;
		FileTimeToSystemTime(&data, &st);
		return
			QString{
			QString::number(st.wDay) + "." +
			QString::number(st.wMonth) + "." +
			QString::number(st.wYear) + " " +
			WEEKS.at(st.wDayOfWeek) + " " +
			QString::number(st.wHour + 3) + ":" +
			QString::number(st.wMinute) + ":" +
			QString::number(st.wSecond) + "." +
			QString::number(st.wMilliseconds)
		};
	}
	template<typename In>
	u64 to_filetime(const In& time)
	{
		if constexpr (std::is_same_v<In, QDateTime>)
		{
			QDateTime origin(QDate(1601, 1, 1), QTime(0, 0, 0, 0), Qt::UTC);
			return 10000 * origin.msecsTo(time) + 1;
		}
		else if constexpr (std::is_same_v<In, FILETIME>)
		{
			return time.dwLowDateTime | static_cast<u64>(time.dwHighDateTime) << 32;
		}
		else if constexpr (std::is_same_v<In, QString>)
		{
			return get_filetime<u64>(time, int(ct::type::last_write));
		}
		return 0;
	}
	template<typename Out>
	Out get_filetime(const QString& pathFile, const int& type)
	{
		HANDLE hFile = CreateFile(pathFile.toStdWString().c_str(),
								  GENERIC_READ,
								  FILE_SHARE_READ,
								  NULL,
								  OPEN_EXISTING,
								  FILE_ATTRIBUTE_NORMAL,
								  NULL);
		FILETIME       CreationTime;
		FILETIME       LastAccessTime;
		FILETIME       LastWriteTime;

		memset(&CreationTime, 0, sizeof(CreationTime));
		memset(&LastAccessTime, 0, sizeof(LastAccessTime));
		memset(&LastWriteTime, 0, sizeof(LastWriteTime));

		if (!::GetFileTime(hFile, &CreationTime, &LastAccessTime, &LastWriteTime))
		{
			return 0;
		}

		if constexpr (std::is_same_v<Out, QString>)
		{
			return type == static_cast<int>(type::last_write) ?
				from_filetime(LastWriteTime) :
				(type == static_cast<int>(type::last_access) ? 
				 from_filetime(LastAccessTime) : 
				 from_filetime(CreationTime));
		}
		else if constexpr (std::is_same_v<Out, u64>)
		{
			return type == static_cast<int>(type::last_write) ?
				to_filetime(LastWriteTime) :
				(type == static_cast<int>(type::last_access) ? 
				 to_filetime(LastAccessTime) : 
				 to_filetime(CreationTime));
		}
		else if constexpr (std::is_same_v<Out, FILETIME>)
		{
			return type == static_cast<int>(type::last_write) ?
				LastWriteTime :
				(type == static_cast<int>(type::last_access) ? 
				 LastAccessTime : 
				 CreationTime);
		}
		return 0;
	}
};