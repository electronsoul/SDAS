from pyroutelib3 import Router

def routeWay():
    print("Routing...")
    router = Router("car", "map.osm")
    routeLatLons = "No Way"
    start = router.findNode(28.40375,76.95810)
    end = router.findNode(28.40471,76.95700)
    status, route = router.doRoute(start, end)
    if status == 'success':
        routeLatLons = list(map(router.nodeLatLon, route))
    return routeLatLons


if __name__ == '__main__':
    print(routeWay())


