CREATE TABLE IF NOT EXISTS TestTable (_id INTEGER PRIMARY KEY AUTOINCREMENT, platform TEXT, i INTEGER, f FLOAT);

INSERT OR IGNORE INTO TestTable (platform, i, f)
VALUES
('iOS', 1, 0.111111111),
('Android', 2, 0.2222222222),
('HarmonyOS', 3, 0.3333333333),
('Windows', 4, 0.4444444444),
('macOS', 5, 0.5555555555),
('Ubuntu', 6, 0.6666666666),
('PlayStation', 7, 0.7777777777),
('Switch', 8, 0.8888888888),
('XBox', 9, 0.9999999999);
