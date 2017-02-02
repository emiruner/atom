    inline bool isInvalid() const
    {
        return TokenType::Invalid == type_;
    }

    inline bool isEof() const
    {
        return TokenType::Eof == type_;
    }

    inline bool isComma() const
    {
        return TokenType::Comma == type_;
    }

    inline bool isLSquare() const
    {
        return TokenType::LSquare == type_;
    }

    inline bool isRSquare() const
    {
        return TokenType::RSquare == type_;
    }

    inline bool isGt() const
    {
        return TokenType::Gt == type_;
    }

    inline bool isStringLiteral() const
    {
        return TokenType::StringLiteral == type_;
    }

    inline bool isIntegerLiteral() const
    {
        return TokenType::IntegerLiteral == type_;
    }

    inline bool isIdentifier() const
    {
        return TokenType::Identifier == type_;
    }

    inline bool isStore() const
    {
        return TokenType::Store == type_;
    }

    inline bool isIn() const
    {
        return TokenType::In == type_;
    }

    inline bool isTo() const
    {
        return TokenType::To == type_;
    }

    inline bool isWith() const
    {
        return TokenType::With == type_;
    }

    inline bool isAat() const
    {
        return TokenType::Aat == type_;
    }

    inline bool isAlen() const
    {
        return TokenType::Alen == type_;
    }

    inline bool isAput() const
    {
        return TokenType::Aput == type_;
    }

    inline bool isCroa() const
    {
        return TokenType::Croa == type_;
    }

    inline bool isCro() const
    {
        return TokenType::Cro == type_;
    }

    inline bool isSend() const
    {
        return TokenType::Send == type_;
    }

    inline bool isIf1() const
    {
        return TokenType::If1 == type_;
    }

    inline bool isIfnot1() const
    {
        return TokenType::Ifnot1 == type_;
    }

    inline bool isRet() const
    {
        return TokenType::Ret == type_;
    }

    inline bool isRetRes() const
    {
        return TokenType::RetRes == type_;
    }

    inline bool isLCurly() const
    {
        return TokenType::LCurly == type_;
    }

    inline bool isRCurly() const
    {
        return TokenType::RCurly == type_;
    }

    inline bool isSemicolon() const
    {
        return TokenType::Semicolon == type_;
    }

    inline bool isFloatLiteral() const
    {
        return TokenType::FloatLiteral == type_;
    }

    inline bool isSimplefnRaw() const
    {
        return TokenType::SimplefnRaw == type_;
    }

    inline bool isSimplefn() const
    {
        return TokenType::Simplefn == type_;
    }

    inline bool isObjarr() const
    {
        return TokenType::Objarr == type_;
    }

    inline bool isObject() const
    {
        return TokenType::Object == type_;
    }

    inline bool isBytearr() const
    {
        return TokenType::Bytearr == type_;
    }

    inline bool isColon() const
    {
        return TokenType::Colon == type_;
    }

    inline bool isEqual() const
    {
        return TokenType::Equal == type_;
    }

    inline bool isVar() const
    {
        return TokenType::Var == type_;
    }

    inline bool isHalt() const
    {
        return TokenType::Halt == type_;
    }

    inline bool isIexh() const
    {
        return TokenType::Iexh == type_;
    }

    inline bool isRaise() const
    {
        return TokenType::Raise == type_;
    }

    inline bool isJmp() const
    {
        return TokenType::Jmp == type_;
    }

    inline bool isSet() const
    {
        return TokenType::Set == type_;
    }

