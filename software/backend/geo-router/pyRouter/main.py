from pyroutelib3 import Router


def routeWay():
    print("Routing...")
    router = Router("car")  # , "map.osm")
    routeLatLons = "No Way"
    start = router.findNode(28.377860092388858, 76.90486002693952)
    end = router.findNode(28.378295478982903, 76.89937285016866)
    status, route = router.doRoute(start, end)
    if status == 'success':
        routeLatLons = list(map(router.nodeLatLon, route))
    return routeLatLons


if __name__ == '__main__':
    routeList = routeWay()
    if routeList.__eq__("No Way"):
        print("No Way")
    else:
        strg = " "
        for i in range(0, routeList.__len__()):
            with (open("example.txt", "a") as file):
                strg = "{ \"index\":" + f"{i}" + ", \"lat\":" + f"{routeList[i][0]}" + ", \"lon\":" + f"{routeList[i][1]}" + ", \"speed\":" + f"{60}" + " },\"" + "\r"
                print(routeList[i])
                file.write(strg)
                file.close()
