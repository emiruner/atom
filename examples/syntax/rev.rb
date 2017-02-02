def nothing = {};

def streamSplitter = stdobject {
    field :stream;
    
    method :newOnStream => stream => {
        def n = self :clone;

        n :set :stream stream;
	n
    };

    method :nextWord => {
        var ch;
        var word = '';
        var inWord = false;

        {
            ch = self :stream :nextChar;

            ch == -1 :or {ch :isWhiteSpace :and {inWord}} :then {
                ^ ( word == '' :then { nothing } :else { word })
            };

            ch :isWhiteSpace :not :then {
                inWord = true;
                word = word + ch;
            };
	} :loop;
    };
};

def ss = streamSplitter :newOnStream stdin;

var word;

{ (word = ss :nextWord) <> nothing } :whileTrue {
    console (:println, word :reverse)
};

