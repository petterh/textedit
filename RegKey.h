/*
 * $Header: /Book/RegKey.h 2     4.06.02 13:40 Oslph312 $
 *
 * Auto-closing registry keys.
 */

class RegKey {
private:
    HKEY _hkey;

public:
    RegKey( HKEY hkey ) : _hkey( hkey ) {
    }
    ~RegKey() {
        verify( !isValid() || ERROR_SUCCESS == RegCloseKey( _hkey ) );
    }
    operator HKEY() const {
        return _hkey;
    }
    bool isValid() const {
        return 0 != _hkey;
    }
};

// end of file
