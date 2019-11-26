#pragma once

template <typename T, unsigned Length>
inline unsigned ArraySize(const T (&v)[Length])
{
	return Length;
}
