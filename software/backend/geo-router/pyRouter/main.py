from pyroutelib3 import Router


def routeWay():
    print("Routing...")
    router = Router("car")  # , "map.osm")
    routeLatLons = "No Way"
    start = router.findNode(28.406749149798358, 76.95745293210388)
    end = router.findNode(28.39945159401554, 76.9607989983696)
    status, route = router.doRoute(start, end)
    if status == 'success':
        routeLatLons = list(map(router.nodeLatLon, route))
    return routeLatLons


if __name__ == '__main__':
    routeList = routeWay()
    datapair = {}
    strg = " "
    for i in range(0, 43):
        with (open("example.txt", "a") as file):
            strg = "{ \"index\":" + f"{i}" + ", \"lat\":" + f"{routeList[i][0]}" + ", \"lon\":" + f"{routeList[i][1]}" + ", \"speed\":" + f"{60}" + " },\"" + "\r"
            print(strg)
            file.write(strg)
            file.close()
