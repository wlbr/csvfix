//----------------------------------------------------------------------------
// a_matrix.h
//
// simple 2d matrix
//
// Copyright (C) 20010 Neil Butterworth
//----------------------------------------------------------------------------


#ifndef INC_A_MATRIX_H
#define INC_A_MATRIX_H

#include <vector>
#include <stdexcept>
#include <ostream>

namespace ALib {

//----------------------------------------------------------------------------

template <typename T>
class Matrix2D {

	public:

		Matrix2D( unsigned int  width, unsigned int  height,
							const T & v  = T() ) {
			if ( width == 0 || height == 0 ) {
				throw std::out_of_range( "Invalid Matrix2D size ");
			}
			for ( unsigned int x = 0; x < width; x++ ) {
				mData.push_back( std::vector<T>( height, v ) );
			}
		}

		T & operator()( unsigned int x, unsigned int y ) {
			if ( x >= Width() || y >= Height() ) {
				throw std::range_error( "Invalid Matrix2D index" );
			}
			return mData[x][y];
		}

		const T & operator()( unsigned int x, unsigned int y ) const {
			if ( x >= Width() || y >= Height() ) {
				throw std::range_error( "Invalid Matrix2D index" );
			}
			return mData[x][y];
		}


		void Clear( const T & v  = T() ) {
			for ( unsigned int x = 0; x < Width(); x++ ) {
				for ( unsigned int y = 0; y < Height(); y++ ) {
					mData[x][y] = v;
				}
			}
		}

		unsigned int Width() const {
			return mData.size();
		}

		unsigned int Height() const {
			return mData[0].size();
		}

		void DumpOn( std::ostream & os ) {
			for ( unsigned int y = 0; y < Height(); y++ ) {
				for ( unsigned int x = 0; x < Width(); x++ ) {
					os << '[' << mData[x][y] << ']';
				}
				os << "\n";
			}
		}

	private:

		std::vector <std::vector  <T> > mData;
};

//----------------------------------------------------------------------------

} // namespace

#endif
