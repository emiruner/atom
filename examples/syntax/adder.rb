def console = loader :required 'system-console';

def promptForNumber = prompt => {
    console :println prompt;
    console :readNumber
};

def main = {
    def x = promptForNumber :$ 'Input first number: ';
    def y = promptForNumber :$ 'Input second number: ';
    
    console :println (x, ' + ', y, ' = ', x + y)
};
    
main :$;

