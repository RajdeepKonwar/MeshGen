all:
	$(MAKE) -C ./GeoGen
	$(MAKE) -C ./EurekaGen

clean:
	$(MAKE) -C ./GeoGen clean
	$(MAKE) -C ./EurekaGen clean
