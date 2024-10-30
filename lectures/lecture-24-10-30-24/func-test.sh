# Is a shebang required?

theTestFunc () {
    echo 'This is a test function showing $1'
    echo 'Maybe maybe $1' $1
    echo "Well, well, well $1"
    echo $2
}

if [ $# -ne 3 ]; then
    echo 'Usage: func-test.sh arg1 arg2 arg3'
    exit 1
fi

theTestFunc $1 $2
theTestFunc $3


