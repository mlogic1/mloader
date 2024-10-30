#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <exception>

namespace autolog
{
	constexpr const char* DB_MODEL_CREATE_STATEMENT = ""
	"CREATE TABLE AUTOLOG_META"
	"("
	"	Tag   TEXT,"
	"	Value TEXT"
	");"
	"CREATE TABLE Vehicle"
	"("
	"	ID INTEGER PRIMARY KEY AUTOINCREMENT,"
	"	Make TEXT,"
	"	Model TEXT,"
	"	Year INTEGER,"
	"	Plate TEXT"
	");"
	"CREATE TABLE VehicleService"
	"("
	"	ID INTEGER PRIMARY KEY AUTOINCREMENT,"
	"	VehicleID	INT NOT NULL,"
	"	Description TEXT,"
	"	Date        TEXT,"
	"	Cost        REAL,"
	"	FOREIGN KEY (VehicleId) REFERENCES Vehicle(Id)"
	");"
	"CREATE TABLE VehicleOdometer"
	"("
	"	ID INTEGER PRIMARY KEY AUTOINCREMENT,"
	"	VehicleID	INT NOT NULL,"
	"	Odometer	INTEGER,"
	"	Date		TEXT,"
	"	FOREIGN KEY (VehicleId) REFERENCES Vehicle(Id)"
	");"
	"CREATE TABLE VehicleImage"
	"("
	"	ID INTEGER PRIMARY KEY AUTOINCREMENT,"
	"	VehicleID	INT NOT NULL,"
	"	Image		BLOB,"
	"	ImageFormat TEXT,"
	"	FOREIGN KEY (VehicleId) REFERENCES Vehicle(Id)"
	");"
	"CREATE TABLE VehicleServiceImage"
	"("
	"	ID INTEGER PRIMARY KEY AUTOINCREMENT,"
	"	VehicleServiceID	INT NOT NULL,"
	"	Image				BLOB,"
	"	ImageFormat 		TEXT,"
	"	FOREIGN KEY (VehicleServiceID) REFERENCES VehicleService(Id)"
	");";
}