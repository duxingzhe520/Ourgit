# OurGit

## .OurGit
/objects(文件名是哈希值，文件内容是OurGitObject序列化后的std::string内容)
（存放Blob, Tree, Commit, Index）


/refs
    HEAD: 存放当前指向的Commit的哈希值
    branch: 先暂时一直存main
    
Index: 存放暂存区的序列化后的std::string内容


