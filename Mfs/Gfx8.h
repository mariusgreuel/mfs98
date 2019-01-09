
#ifdef __cplusplus
  extern "C" {
#endif

extern void _InitGfx8( LPVOID lpddsf, DWORD Pitch );
extern void _SetColor8( DWORD Color );
extern void _MoveTo8( DWORD x, DWORD y );
extern void _LineTo8( DWORD x, DWORD y );
extern void _Polyline8( CONST POINT *lppt, DWORD cPoints );
extern void _Polygon8( CONST POINT *lppt, DWORD cPoints );
extern void _TPolygon8( CONST POINT *lppt, DWORD cPoints );

#ifdef __cplusplus
};
#endif

