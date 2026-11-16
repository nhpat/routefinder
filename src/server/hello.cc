#include <iostream>

#include <grpcpp/grpcpp.h>
#include <s2/s2cell_id.h>
#include <s2/s2latlng.h>

int main() {
  const S2LatLng penn_station = S2LatLng::FromDegrees(40.7506, -73.9935);
  const S2CellId cell(penn_station.ToPoint());

  std::cout << "Penn Station S2 leaf cell: " << cell.ToToken() << "\n";
  std::cout << "gRPC version: " << grpc::Version() << "\n";
  return 0;
}
