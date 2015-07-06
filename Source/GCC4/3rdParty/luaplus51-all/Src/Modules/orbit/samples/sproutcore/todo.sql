CREATE TABLE todo_list
       ("id" INTEGER PRIMARY KEY NOT NULL,
       "description" VARCHAR(255) DEFAULT "",
       "is_done" BOOLEAN DEFAULT "f",
       "created_at" DATETIME);
