PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE students (uuid string PRIMARY KEY, code int, timeout decimal, attempts int,codeSent int);
INSERT INTO "students" VALUES ('11111111-2222-3333-4444-555555555555', NULL,0.0,0,0);
COMMIT;
