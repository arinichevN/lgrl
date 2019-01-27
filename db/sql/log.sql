CREATE TABLE "v_real" (
    "id" INTEGER NOT NULL,
    "mark" INTEGER NOT NULL,
    "value" REAL NOT NULL,
    "state" INTEGER NOT NULL
);
CREATE TABLE "alert" (
    "mark" INTEGER NOT NULL,
    "message" TEXT NOT NULL
);
CREATE UNIQUE INDEX "v_real_ind" on v_real (id ASC, mark ASC);
