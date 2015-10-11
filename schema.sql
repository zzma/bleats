PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE students (uuid string PRIMARY KEY, firstName string, lastName string, uin int);
INSERT INTO "students" VALUES('11111111-2222-3333-4444-555555555555','Bob','King',238492012);
COMMIT;
