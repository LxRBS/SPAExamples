#! /bin/bash

# 本脚本用来给定ll文件，生成dot与png文件，每个函数一个；再生成一个callgraph文件

# 判断参数数量，必须是1个参数
if [ $# -eq 0 ];then
    echo "Execution Failure!"
    echo "There must be ONE argument, but NO argument found."
    exit 1
elif [ $# -gt 1 ];then
    echo "Execution Failure!"
    echo "There must be ONE argument, but $# arguments found."
    exit 1
fi

# 获取文件名以及后缀名
full_filename=$1
filename="${full_filename%.*}"
extension="${full_filename##*.}"

# 必须是ll文件
if [ "$extension" != "ll" ]; then
    echo "Execution Failure!"
    echo "Only dot file is available, but $full_filename is not a dot file."
    exit 1
fi

# 生成cfg的dot文件，每个函数一个
opt -dot-cfg "$full_filename" > /dev/null

# 查找文件中所有以点开头的dot文件
dotfiles=$(ls -a)
for dotfilename in $dotfiles
do
    start=${dotfilename:0:1}
    end=${dotfilename##*.}
    # 如果开头是点，结尾是dot，那就是刚生成的
    if [ "$start" == "." ] && [ "$end" == "dot" ]
    then
        echo "dealing with ""$dotfilename"
        # 首先改名字，然后生成png文件
        good_name="$filename""_""${dotfilename#.}"
        mv $dotfilename $good_name
        dot -Tpng -o "${good_name%.*}"".png" "$good_name"
    fi
done

# 生成callgraph的dot文件，重定向是为了忽略一些警告信息
echo "Generating callgraph.dot"
opt -dot-callgraph "$full_filename" > /dev/null

# 目前生成的名称均为callgraph.dot，改一个名字，命名格式为:本名_callgraph.dot
default_callgraph_name="callgraph.dot"
mv "$default_callgraph_name" "$filename""_callgraph.dot"

# 生成同名的png文件
dot -Tpng -o "$filename""_callgraph.png" "$filename""_callgraph.dot"



