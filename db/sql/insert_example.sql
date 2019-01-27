INSERT OR REPLACE INTO "peer" VALUES
('gwu18_1',49161,'127.0.0.1'),
('gwu22_1',49162,'127.0.0.1'),
('gwu18_2',49161,'127.0.0.1'),
('gwu22_2',49162,'127.0.0.1'),
('regonf_1',49191,'127.0.0.1'),
('gwu74_1',49163,'127.0.0.1'),
('lck_1',49175,'127.0.0.1'),
('lgr_1',49172,'127.0.0.1'),
('gwu59_1',49164,'127.0.0.1'),
('alp_1',49171,'127.0.0.1'),
('alr_1',49174,'127.0.0.1'),
('regsmp_1',49192,'127.0.0.1'),
('stp_1',49179,'127.0.0.1'),
('obj_1',49178,'127.0.0.1'),
('swr_1',49183,'127.0.0.1'),
('swf_1',49182,'127.0.0.1');

INSERT OR REPLACE INTO "remote_channel" (id,peer_id,channel_id) VALUES
(1,'obj_1',1),
(2,'obj_1',2),
(3,'obj_1',3),
(4,'obj_1',4);

INSERT OR REPLACE INTO "prog" (id,kind,interval_sec,max_rows,clear) VALUES
(1, 'fts', 60, 720, 1),
(2, 'fts', 60, 720, 1),
(3, 'fts', 60, 720, 1),
(4, 'fts', 60, 720, 1);

INSERT OR REPLACE INTO "channel" (id,description,prog_id,sensor_remote_channel_id,cycle_duration_sec,cycle_duration_nsec,save,enable,load) VALUES
(1, 'канал1', 1,1, 1,0, 1,1,1),
(2, 'канал2', 1,2, 1,0, 1,1,1),
(3, 'канал3', 1,3, 1,0, 1,1,1),
(4, 'канал4', 1,4, 1,0, 1,1,1);
