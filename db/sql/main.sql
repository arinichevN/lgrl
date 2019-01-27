CREATE TABLE "peer" (
    "id" TEXT NOT NULL,
    "port" INTEGER NOT NULL,
    "ip_addr" TEXT NOT NULL
);
CREATE TABLE "remote_channel"
(
  "id" INTEGER PRIMARY KEY,
  "peer_id" TEXT NOT NULL,
  "channel_id" INTEGER NOT NULL
);
CREATE TABLE "prog" (
    "id" INTEGER PRIMARY KEY,
    "kind" TEXT NOT NULL,
    "interval_sec" INTEGER NOT NULL,
    "max_rows" INTEGER NOT NULL,
    "clear" INTEGER NOT NULL
);
CREATE TABLE "channel" (
    "id" INTEGER PRIMARY KEY,
    "description" TEXT NOT NULL,
    "prog_id" INTEGER NOT NULL,
    "sensor_remote_channel_id" INTEGER NOT NULL,
    "cycle_duration_sec" INTEGER NOT NULL,
    "cycle_duration_nsec" INTEGER NOT NULL,
    "save" INTEGER NOT NULL,
    "enable" INTEGER NOT NULL,
    "load" INTEGER NOT NULL
);
